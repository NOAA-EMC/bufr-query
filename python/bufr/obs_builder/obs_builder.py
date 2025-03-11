import os
import inspect
import bufr

from ..encoders import netcdf, zarr
from .logger import Logger


FILE_ENCODER_DICT = {'netcdf': netcdf.Encoder,
                     'zarr': zarr.Encoder}

def add_encoder_type(name, encoder):
    FILE_ENCODER_DICT[name] = encoder

def add_main_functions(cls):
    def make_obs_builder(*args, **kwargs):
        return cls(*args, **kwargs)

    def create_obs_group(input_path, mapping_path, category, env, ):
        return cls(mapping_path).create_obs_group(input_path, category, env)

    def create_obs_file(input_path, output_path, mapping_path, type='netcdf', append=False):
        return cls(mapping_path).create_obs_file(input_path, output_path, type, append)

    def default_main():
        import sys
        import time
        import argparse
        from bufr import mpi
        from bufr.obs_builder import Logger

        start_time = time.time()

        mpi.App(sys.argv)
        comm = mpi.Comm("world")

        # Required input arguments
        parser = argparse.ArgumentParser()
        parser.add_argument('input', type=str, help='Input BUFR')
        parser.add_argument('mapping', type=str, help='BUFR2IODA Mapping File')
        parser.add_argument('output', type=str, help='Output NetCDF')

        args = parser.parse_args()
        create_obs_file(args.input, args.mapping, args.output)

        end_time = time.time()
        running_time = end_time - start_time
        Logger(os.path.basename(__file__), comm=comm).info(f'Total running time: {running_time}')

    caller_frame = inspect.stack()[1]
    calling_module = inspect.getmodule(caller_frame.frame)
    calling_module.make_obs_builder = make_obs_builder
    calling_module.create_obs_group = create_obs_group
    calling_module.create_obs_file = create_obs_file
    calling_module.default_main = default_main

    if calling_module.__name__ == '__main__':
        default_main()


class ObsBuilder:
    def __init__(self, *args, **kwargs):
        self.map_dict = {}

        log_name = 'obs_builder'

        ERR_MSG = 'ObsBuilder.__init__ has the following signatures: \n' \
                    ' 1. (mapping_path:str=\'obs_builder\'\n' \
                    ' 2. (mapping_path:str, log_name:str=\'obs_builder\')\n' \
                    ' 3. (map_dict:dict[str:str], log_name:str=\'obs_builder\')\n' \
                    '     where map_dict = {\'obs_type\': mapping_path}'

        assert len(args) == 1 or len(args) == 2, ERR_MSG

        if type(args[0]) == str:
            self.map_dict = {'': args[0]}

            if len(args) == 2:
                assert type(args[1]) == str, ERR_MSG
                log_name = args[1]

            else:
                assert False, ERR_MSG

        elif type(args[0]) == dict:
            # Validate the dictionary
            for key, value in args[0].items():
                assert type(key) == str, ERR_MSG
                assert type(value) == str, ERR_MSG

            self.map_dict = args[0]

            if len(args) == 2:
                assert type(args[1]) == str, ERR_MSG
                log_name = args[1]

        else:
            assert False, ERR_MSG

        # kwargs
        if 'log_name' in kwargs:
            assert type(kwargs['log_name']) == str, ERR_MSG
            log_name = kwargs['log_name']

        self.log = Logger(log_name)

    # Virtual Method
    def make_description(self) -> bufr.encoders.Description:
        assert len(self.map_dict) > 0, 'No mapping file provided, please override make_description()'

        return bufr.encoders.Description(list(self.map_dict.values())[0])

    # Virtual Method
    def make_obs(self, comm, input) -> bufr.DataContainer:
        assert len(self.map_dict) > 0, 'Must override _make_obs(), or provide input_dict'
        assert type(input) == str, 'Input was not a path str, please override make_obs'

        mapping_path = self.map_dict.values()[0]
        container = bufr.Parser(input, mapping_path).parse(comm)

        for idx, mapping_path in enumerate(self.map_dict.items()):
            if idx == 0:
                continue

            container.append(bufr.Parser(input, mapping_path).parse(comm))

        return container

    def create_obs_group(self, input, category, env):
        from pyioda.ioda.Engines.Bufr import Encoder as iodaEncoder
        assert type(input) == str, 'Input was not a path str, please override create_obs_group'

        comm = bufr.mpi.Comm(env["comm_name"])
        self.log.comm = comm

        cache_input_path = input
        cache_mapping_path = self.map_dict.values()[0]

        # Check the cache for the data and return it if it exists
        self.log.debug(f'Check if bufr.DataCache exists? \
                         {bufr.DataCache.has(cache_input_path, cache_mapping_path)}')
        if bufr.DataCache.has(cache_input_path, cache_mapping_path):
            container = bufr.DataCache.get(cache_input_path, cache_mapping_path)
            self.log.info(f'Encode {category} from cache')
            data = iodaEncoder(self.make_description()).encode(container)[(category,)]
            self.log.info(f'Mark {category} as finished in the cache')
            bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, [category])
            self.log.info(f'Return the encoded data for {category}')
            return data

        container = self.make_obs(comm, input)

        # Gather data from all tasks into all tasks. Each task will have the complete record
        self.log.info(f'Gather data from all tasks into all tasks')
        container.all_gather(comm)

        self.log.info(f'Add container to cache')
        # Add the container to the cache
        bufr.DataCache.add(cache_input_path,
                           cache_mapping_path,
                           container.all_sub_categories(),
                           container)

        # Encode the data
        self.log.info(f'Encode {category}')
        data = iodaEncoder(self.make_description()).encode(container)[(category,)]

        self.log.info(f'Mark {category} as finished in the cache')
        # Mark the data as finished in the cache
        bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, [category])

        self.log.info(f'Return the encoded data for {category}')
        return data

    def create_obs_file(self, input, output_path, type='netcdf', append=False):

        comm = bufr.mpi.Comm("world")
        self.log.comm = comm

        container = self.make_obs(comm, input)
        container.gather(comm)

        # Encode the data
        if comm.rank() == 0:
            FILE_ENCODER_DICT[type](self.make_description()).encode(container, output_path, append)

        self.log.info(f'Return the encoded data')
