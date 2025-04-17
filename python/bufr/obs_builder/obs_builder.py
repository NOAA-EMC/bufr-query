import json
import os
import inspect
from typing import Union

import bufr

from ..encoders import netcdf, zarr
from .logger import Logger



FILE_ENCODER_DICT = {'netcdf': netcdf.Encoder,
                     'zarr': zarr.Encoder}

def add_encoder_type(name, encoder):
    FILE_ENCODER_DICT[name] = encoder


def add_main_functions(cls, uses_categories=False, uses_cache=False):
    def make_obs_builder(config:dict=None):
        if 'config' in inspect.signature(cls.__init__).parameters:
            return cls(config=config) if config else cls()
        else:
            return cls()

    # Create ObsGroup functions
    def create_obs_group_w_cache(input_path, category, env, config:dict=None):
        return make_obs_builder(config=config).create_obs_group_w_cache(input_path, category, env)

    def create_obs_group_no_cache_cat(input_path, category, env, config:dict=None):
        return make_obs_builder(config=config).create_obs_group_no_cache(input_path, env, category)

    def create_obs_group_no_cache_no_cat(input_path, env, config:dict=None):
        return make_obs_builder(config=config).create_obs_group_no_cache(input_path, env, '')

    def create_obs_file(input_path, output_path, type='netcdf', append=False, config:dict=None):
        make_obs_builder(config=config).create_obs_file(input_path, output_path, type, append)

    def create_obs_file_from_config(config):
        # Get parameters from configuration
        data_format = config["data_format"]
        data_type = config["data_type"]
        cycle_type = config["cycle_type"]
        dump_dir = config["dump_directory"]
        cycle_datetime = config["cycle_datetime"]
        ioda_dir = config["ioda_directory"]

        # Make input path
        yyyymmdd = cycle_datetime[0:8]
        hh = cycle_datetime[8:10]
        bufrfile = f"{cycle_datetime}-{cycle_type}.t{hh}z.{data_format}.tm00.bufr_d"
        input_path = os.path.join(dump_dir, bufrfile)

        # Make output path
        iodafile = f"{cycle_type}.t{hh}z.{data_type}.tm00.nc"
        output_path = os.path.join(ioda_dir, iodafile)

        create_obs_file(input_path, output_path, config=config)

    def default_main():
        import sys
        import time
        import argparse
        import yaml
        from bufr import mpi
        from bufr.obs_builder import Logger

        logger = Logger(os.path.basename(__file__))

        start_time = time.time()

        mpi.App(sys.argv)
        comm = mpi.Comm("world")

        # Required input arguments
        parser = argparse.ArgumentParser()
        parser.add_argument('--input', type=str, help='Input BUFR')
        parser.add_argument('--output', type=str, help='Output NetCDF')
        parser.add_argument('--config', type=str, help='GDAS App style config')

        args = parser.parse_args()

        if args.config:
            with open(args.config, "r") as file:
                config = yaml.safe_load(file)

            create_obs_file_from_config(config)

            if args.output or args.input:
                logger.warning('Ignoring input and output arguments when using config.')
        else:
            if not args.input or not args.output:
                logger.error('Both Input and output arguments are required.')
                sys.exit(1)

            create_obs_file(args.input, args.output)

        end_time = time.time()
        running_time = end_time - start_time
        logger.info(f'Total running time: {running_time}')

    caller_frame = inspect.stack()[1]
    calling_module = inspect.getmodule(caller_frame.frame)
    calling_module.make_obs_builder = make_obs_builder

    if uses_cache:
        if uses_categories:
            calling_module.create_obs_group = create_obs_group_w_cache
        else:
            assert False, 'Caching is only supported with categories'
    else:
        if uses_categories:
            calling_module.create_obs_group = create_obs_group_no_cache_cat
        else:
            calling_module.create_obs_group = create_obs_group_no_cache_no_cat

    calling_module.create_obs_file = create_obs_file
    calling_module.create_obs_file_from_config = create_obs_file_from_config
    calling_module.default_main = default_main

    if calling_module.__name__ == '__main__':
        default_main()


class ObsBuilder:
    def __init__(self, mapping_path:Union[str, dict], config:dict=None, log_name:str='obs_builder'):
        """
        ObsBuilder constructor

        Args:
            mapping_path (Union[str, dict]): Path to the mapping file or a dictionary of mapping paths
            config (dict): Configuration dictionary
            log_name (str): Name of the logger object
        """

        self.map_dict = {}

        if isinstance(mapping_path, str):
            self.map_dict[''] = mapping_path
        elif isinstance(mapping_path, dict):
            self.map_dict = mapping_path

        self.log = Logger(log_name)
        self.config = config
        self.description = self._make_description()

    # Virtual Method
    def make_obs(self, comm, input : Union[str, dict]) -> bufr.DataContainer:
        if not isinstance(input, str) or len(self.map_dict) != 1:
            assert False, 'You must create a custom override for make_obs().'

        mapping_path = list(self.map_dict.values())[0]
        container = bufr.Parser(input, mapping_path).parse(comm)

        for idx, mapping_path in enumerate(self.map_dict.items()):
            if idx == 0:
                continue

            container.append(bufr.Parser(input, mapping_path).parse(comm))

        return container

    def _make_description(self) -> bufr.encoders.Description:
        assert len(self.map_dict) > 0, 'No mapping file provided, please override _make_description()'

        return bufr.encoders.Description(list(self.map_dict.values())[0])

    def create_obs_group_w_cache(self, input, category, env):
        from pyioda.ioda.Engines.Bufr import Encoder as iodaEncoder
        assert type(input) == str, 'Input was not a path str, please override create_obs_group'

        comm = bufr.mpi.Comm(env["comm_name"])
        self.log.comm = comm

        cache_input_path = input
        cache_mapping_path = list(self.map_dict.values())[0]

        # Check the cache for the data and return it if it exists
        self.log.debug(f'Check if bufr.DataCache exists? \
                         {bufr.DataCache.has(cache_input_path, cache_mapping_path)}')
        if bufr.DataCache.has(cache_input_path, cache_mapping_path):
            container = bufr.DataCache.get(cache_input_path, cache_mapping_path)
            self.log.info(f'Encode {category} from cache')
            data = iodaEncoder(self.description).encode(container)[(category,)]
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
        data = iodaEncoder(self.description).encode(container)[(category,)]

        self.log.info(f'Mark {category} as finished in the cache')
        # Mark the data as finished in the cache
        bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, [category])

        self.log.info(f'Return the encoded data for {category}')
        return data

    def create_obs_group_no_cache(self, input, env, category=''):
        from pyioda.ioda.Engines.Bufr import Encoder as iodaEncoder
        assert type(input) == str, 'Input was not a path str, please override create_obs_group'

        comm = bufr.mpi.Comm(env["comm_name"])
        self.log.comm = comm

        container = self.make_obs(comm, input)
        container.gather(comm)

        # Encode the data
        if category == '':
            self.log.info(f'Encoding')
            data = next(iter(iodaEncoder(self.description).encode(container).values()))
        else:
            self.log.info(f'Encoding {category}')
            data = iodaEncoder(self.description).encode(container)[(category,)]

        return data

    def create_obs_file(self, input, output_path, type='netcdf', append=False):

        comm = bufr.mpi.Comm("world")
        self.log.comm = comm

        container = self.make_obs(comm, input)
        container.gather(comm)

        # Encode the data
        if comm.rank() == 0:
            FILE_ENCODER_DICT[type](self.description).encode(container, output_path, append)

        self.log.info(f'Return the encoded data')
