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
    def create_obs_group(input_path, mapping_path, category, env):
        obs_builder = cls(input_path, mapping_path)
        obs_builder.create_obs_group(category, env)

    def create_obs_file(input_path, mapping_path, output_path, type='netcdf', append=False):
        obs_builder = cls(input_path, mapping_path)
        return obs_builder.create_obs_file(output_path, type, append)

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
    calling_module.create_obs_group = create_obs_group
    calling_module.create_obs_file = create_obs_file
    calling_module.default_main = default_main

    if calling_module.__name__ == '__main__':
        default_main()


class ObsBuilder:
    def __init__(self, *args, **kwargs):
        self.input_dict = {}

        log_name = 'obs_builder'

        ERR_MSG = 'ObsBuilder.__init__ has the following signatures: \n' \
                    ' 1. (log_name:str=\'obs_builder\'\n' \
                    ' 2. (input_path:str, mapping_path:str, log_name:str=\'obs_builder\')\n' \
                    ' 3. (input_dict:dict[str:(str, str)], log_name:str=\'obs_builder\')\n' \
                    '     where input_dict = {\'obs_type\': (input_path, mapping_path)}'

        if len(args) == 0:
            return # Create empty object

        if type(args[0]) == str:
            if len(args) == 1:
                log_name = args[0]

            elif len(args) >= 2:
                assert len(args) >= 2 and type(args[1]) == str, ERR_MSG
                self.input_dict = {'', (args[0], args[1])}

                if len(args) == 3:
                    assert type(args[2]) == str, ERR_MSG
                    log_name = args[2]

        elif type(args[0]) == dict:
            # Validate the dictionary
            for key, value in args[0].items():
                assert type(key) == str, ERR_MSG
                assert type(value) == tuple and len(value) == 2, ERR_MSG
                assert type(value[0]) == str and type(value[1]) == str, ERR_MSG

            self.input_dict = args[0]

            if len(args) == 2:
                assert type(args[1]) == str, ERR_MSG
                log_name = args[1]

        # kwargs
        if 'log_name' in kwargs:
            assert type(kwargs['log_name']) == str, ERR_MSG
            log_name = kwargs['log_name']

        self.log = Logger(log_name)

    # Virtual Method
    def _make_description(self) -> bufr.encoders.Description:
        assert len(self.input_dict) > 0, 'Must override _make_description(), or provide input_dict'
        _, mapping_path = self.input_dict.values()[0]
        return bufr.encoders.Description(mapping_path)

    # Virtual Method
    def _make_obs(self, comm) -> bufr.DataContainer:
        assert len(self.input_dict) > 0, 'Must override _make_obs(), or provide input_dict'

        input_path, mapping_path = self.input_dict.values()[0]
        container = bufr.Parser(input_path, mapping_path).parse(comm)

        for idx, (key, values) in enumerate(self.input_dict.items()):
            if idx == 0:
                continue

            input_path, mapping_path = values()[0]
            container.append(bufr.Parser(input_path, mapping_path).parse(comm))

        return container

    def create_obs_group(self, category, env):
        from pyioda.ioda.Engines.Bufr import Encoder as iodaEncoder

        comm = bufr.mpi.Comm(env["comm_name"])
        self.log.comm = comm

        cache_input_path = self.input_dict.values()[0][0]
        cache_mapping_path = self.input_dict.values()[0][1]

        # Check the cache for the data and return it if it exists
        self.log.debug(f'Check if bufr.DataCache exists? {bufr.DataCache.has(cache_input_path, 
                                                                             cache_mapping_path)}')
        if bufr.DataCache.has(cache_input_path, cache_mapping_path):
            container = bufr.DataCache.get(cache_input_path, cache_mapping_path)
            self.log.info(f'Encode {category} from cache')
            data = iodaEncoder(self._make_description()).encode(container)[(category,)]
            self.log.info(f'Mark {category} as finished in the cache')
            bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, [category])
            self.log.info(f'Return the encoded data for {category}')
            return data

        container = self._make_obs(comm)

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
        data = iodaEncoder(self._make_description()).encode(container)[(category,)]

        self.log.info(f'Mark {category} as finished in the cache')
        # Mark the data as finished in the cache
        bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, [category])

        self.log.info(f'Return the encoded data for {category}')
        return data

    def create_obs_file(self, output_path, type='netcdf', append=False):

        comm = bufr.mpi.Comm("world")
        self.log.comm = comm

        container = self._make_obs(comm)
        container.gather(comm)

        # Encode the data
        if comm.rank() == 0:
            FILE_ENCODER_DICT[type](self._make_description()).encode(container, output_path, append)

        self.log.info(f'Return the encoded data')
