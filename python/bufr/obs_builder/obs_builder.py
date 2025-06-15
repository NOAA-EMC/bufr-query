import json
import os
import inspect
from typing import Union

import bufr

from ..encoders import netcdf
from .logger import Logger


FILE_ENCODER_DICT = {'netcdf': netcdf.Encoder}

def add_encoder_type(name, encoder):
    FILE_ENCODER_DICT[name] = encoder

class ObsBuilder:
    def __init__(self,
                 mapping_path:Union[str, dict],
                 config:dict=None,
                 log_name:str='obs_builder'):
        """
        ObsBuilder constructor.

        :param mapping_path: Path to the mapping file or a dict[str, str] which maps names to
                             mapping file paths.
        :param config: Configuration dictionary (optional) used to initialize the obs-builder.
        :param log_name: Name for the logger (optional).
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
        """
        This method is the main method that can be overridden (optional). Its objective is to read
        the bufr file and to create an DataContainer object and return it. The data container is
        used in conjunction with the encoder Descriptor to encode the data into the format of your
        choice.

        :param comm: MPI communicator
        :param input: Either a path to the input data or a dictionary the maps the input data to the
                      mapping file paths (see constructor)
        :return: DataContainer object
        """
        if not isinstance(input, str) or len(self.map_dict) != 1:
            raise NotImplementedError('You must create a custom override for make_obs().')

        mapping_path = list(self.map_dict.values())[0]
        container = bufr.Parser(input, mapping_path).parse(comm)

        for idx, mapping_path in enumerate(self.map_dict.items()):
            if idx == 0:
                continue

            container.append(bufr.Parser(input, mapping_path).parse(comm))

        return container

    def _make_description(self) -> bufr.encoders.Description:
        """
        Use this override to extend the encoder description for the data when adding data fields.
        """

        if len(self.map_dict) == 0:
            raise ValueError('No mapping file provided. Either override _make_description() or '
                             'provide a mapping file in the constructor.')

        return bufr.encoders.Description(list(self.map_dict.values())[0])

    def create_obs_file(self, input, output, type='netcdf', append=False):
        """
        Create an observation file from the input data. Override this method if you want to
        customize the file creation process or if you need a different function signature (ex: you
        need to pass multiple input files). add_main_functions will copy the function signature.

        :param input: Input path to the BUFR file.
        :param output: Output file name
        :param type: Data type to encode into (optional)
        :param append: Add to the file if it exists or create a new file. (optional)
        """

        comm = bufr.mpi.Comm("world")
        self.log.comm = comm

        # Create observation container
        container = self.make_obs(comm, input)

        # Gather and encode data 
        rank = comm.rank()
        size = comm.size()
        subcategories = container.all_sub_categories()

        # Container has no category (subcategories=[[]]; This is list with one empty list inside)
        # Empty list is falsy  
        if not subcategories[0] or all(len(sub) == 0 for sub in subcategories): 
            self.log.info("Container with no cagegories defined - encoding the container at rank 0.")
            container.gather(comm)
            if rank == 0:
                FILE_ENCODER_DICT[type](self.description).encode(container, output, append)
        # Container has categories 
        else:
            self.log.info("Container with categories defined - encoding subcategories in parallel.")
            container.all_gather(comm)
            self._encode_by_rank(container, subcategories, output, type, append, rank, size)

        self.log.info(f'Return the encoded data')

    def _encode_by_rank(self, container, subcategories, output, type, append, rank, size):
        """
        Helper function: Encode subcategories in parallel using MPI ranks.
        """
        encoder_class = FILE_ENCODER_DICT[type]

        for i, subcat in enumerate(subcategories):
            if i % size != rank:
                continue  # Skip subcategories not assigned to this rank

            self.log.info_all(f"Rank {rank} encoding subcategory: {subcat}")
            sub_container = container.get_sub_container(subcat)
            encoder = encoder_class(self.description)
            encoder.encode(sub_container, output, append)

    def create_obs_group(self, input, env, category:str=None, cache_categories:list=None):
        """
        Create an observation file from the input data. Override this method if you want to
        customize the file creation process or if you need a different function signature (ex: you
        need to pass multiple input files).

        :param input: Input path to the BUFR file.
        :param env: The IODA environment. Dictionary with keys: start_time, end_time, comm_name
        :param category: The category to encode (comma-separated subcategories). This string is
                         parsed into a tuple of subcategories. (optional)
        :param cache_categories: The list of categories to cache. Each category is a string that
                                 is parsed into a tuple of subcategories. (optional)
        :return: IODA ObsGroup object.
        """

        # Guard Block
        if (cache_categories is not None) and (category is None):
            raise ValueError('Category must be provided if cache_categories are specified.')

        if category:
            if not isinstance(category, str):
                raise ValueError('Category must be a comma separated string of sub-categories '
                                 'ex: \'npp\'.')

        if cache_categories:
            if not isinstance(cache_categories, list) or \
               not len(cache_categories) > 0 or \
               not isinstance(cache_categories[0], str):
                raise ValueError('Cache categories must be a list of categories ex: [\'goes-17\''
                                 ', \'goes-18\'].')

            if category not in cache_categories:
                raise ValueError(f'Category {category} not found in cache categories.')

        # Parse category and cache_categories strings
        if category:
            category = tuple(category.replace(' ', '').split(','))

        if cache_categories:
            cache_categories = [tuple(cat.replace(' ', '').split(',')) for cat in cache_categories]

        if cache_categories:
            return self._create_obs_group_w_cache(input, env, category, cache_categories)
        else:
            return self._create_obs_group_no_cache(input, env, category)

    def _create_obs_group_w_cache(self, input, env, category:tuple, cache_categories:list):
        from pyioda.ioda.Engines.Bufr import Encoder as iodaEncoder

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
            data = iodaEncoder(self.description).encode(container)[category]
            self.log.info(f'Mark {category} as finished in the cache')
            bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, category)
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
                           cache_categories,
                           container)

        # Encode the data
        self.log.info(f'Encode {category}')
        data = iodaEncoder(self.description).encode(container)[category]

        self.log.info(f'Mark {category} as finished in the cache')
        # Mark the data as finished in the cache
        bufr.DataCache.mark_finished(cache_input_path, cache_mapping_path, category)

        self.log.info(f'Return the encoded data for {category}')
        return data

    def _create_obs_group_no_cache(self, input, env, category:tuple = None):
        from pyioda.ioda.Engines.Bufr import Encoder as iodaEncoder

        comm = bufr.mpi.Comm(env["comm_name"])
        self.log.comm = comm

        container = self.make_obs(comm, input)
        container.all_gather(comm)

        # Encode the data
        if not category:
            self.log.info(f'Encoding')
            data = next(iter(iodaEncoder(self.description).encode(container).values()))
        else:
            self.log.info(f'Encoding {category}')
            data = iodaEncoder(self.description).encode(container)[category]

        return data
