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

    def create_obs_file(self, input, output, type='netcdf', append=False):

        comm = bufr.mpi.Comm("world")
        self.log.comm = comm

        container = self.make_obs(comm, input)
        container.gather(comm)

        # Encode the data
        if comm.rank() == 0:
            FILE_ENCODER_DICT[type](self.description).encode(container, output, append)

        self.log.info(f'Return the encoded data')

    def create_obs_group(self, input, env, category=''):
        """
        Create an observation group from the input data.

        Args:
            input (str): Path to the input data.
            env (dict): Environment variables.
            category (str): Category of the observation group.

        Returns:
            dict: Encoded data.
        """
        if category:
            return self._create_obs_group_w_cache(input, category, env)
        else:
            return self._create_obs_group_no_cache(input, env)

    def _create_obs_group_w_cache(self, input, category, env):
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

    def _create_obs_group_no_cache(self, input, env, category=''):
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
