# (C) Copyright 2024 NOAA/NWS/NCEP/EMC
import sys
import os
import subprocess
import netCDF4

import bufr
from bufr.encoders import netcdf
import numpy as np
from bufr.obs_builder import ObsBuilder, add_main_functions, map_path

def is_empty_nc(file_path):
    with netCDF4.Dataset(file_path, 'r') as ds:
        loc_dim = ds.dimensions.get('Location')
        return loc_dim is None or len(loc_dim) == 0

def run_compare(input_path, comp_path):
    if is_empty_nc(input_path) and is_empty_nc(comp_path):
        print(f"[INFO] Skipping compare: both {input_path} and {comp_path} are empty.")
        return
    result = subprocess.Popen(f'nccmp -d -m -g -f -S {input_path} {comp_path}', shell=True).wait()
    assert result == 0, f"Comparison failed for {input_path} and {comp_path}."

MAPPING_PATH = map_path('testinput/bufrtest_mhs_mapping.yaml')

class TestObsBuilder(ObsBuilder):
    def __init__(self):
        super().__init__(MAPPING_PATH)

    def make_obs(self, comm, input):
        # Custom implementation for testing
        return super().make_obs(comm, input)

add_main_functions(TestObsBuilder, execute_main=False)


def test_mpi_basic():
    DATA_PATH = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_mhs_basic_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_mhs_basic_parallel.nc'
    COMP_PATH = 'testoutput/bufrtest_mhs_basic.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        run_compare(OUTPUT_PATH, COMP_PATH)


def test_mpi_categories():
    DATA_PATH = 'testdata/gdas.t12z.esmhs.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_esmhs_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_esmhs_{splits/satId}_cats.nc'
    COMP_PATH = 'testoutput/bufrtest_esmhs_noaa-19.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        run_compare('testrun/bufrtest_esmhs_noaa-19_cats.nc', COMP_PATH)


def test_mpi_sub_container():
    DATA_PATH = 'testdata/gdas.t12z.esmhs.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_esmhs_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_esmhs_{splits/satId}_sub_container.nc'
    COMP_PATH = 'testoutput/bufrtest_esmhs_noaa-19.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container = container.get_sub_container(['noaa-19'])
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        run_compare('testrun/bufrtest_esmhs_noaa-19_sub_container.nc', COMP_PATH)


def test_mpi_all_gather():
    DATA_PATH = 'testdata/gdas.t06z.snocvr.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_long_strs_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_mpi_all_gather.nc'
    COMP_PATH = 'testoutput/bufrtest_long_strs.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.all_gather(comm)

    if comm.rank() == 1:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        run_compare(OUTPUT_PATH, COMP_PATH)

def test_mpi_encoder():

    DATA_PATH = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'
    OUTPUT_PATH = 'testrun/bufrtest_mhs.nc'
    COMP_PATH = 'testoutput/bufrtest_mhs_encoder_parallel.nc'

    bufr.mpi.App(sys.argv)
    comm = bufr.mpi.Comm("world")
    rank = comm.rank()
    size = comm.size()

    obs_builder = TestObsBuilder()
    obs_builder.log.comm = comm

    container = obs_builder.make_obs(comm, DATA_PATH)

    subcategories = container.all_sub_categories()
    obs_builder.log.info(f"subcategories: {subcategories}")

    obs_builder.log.info("Container with categories defined - encoding subcategories in parallel.")
    OUTPUT_PATH = 'testrun/bufrtest_mhs_{splits/satId}.nc'
    container.all_gather(comm)
    obs_builder._encode_by_rank(container, subcategories, OUTPUT_PATH, 'netcdf', False, rank, size)

    # Only rank 0 needs to do the comparison
    if rank == 0:
        for subcat in subcategories:
            cat_str = '_'.join(str(x) for x in subcat)  # e.g., "metop-a"
            output_path = OUTPUT_PATH.replace('{splits/satId}', cat_str)
            comp_path = COMP_PATH.replace('mhs', f'mhs_{cat_str}')  # reference file must follow same naming
            obs_builder.log.info(f"Comparing {output_path} with {comp_path}")
            run_compare(output_path, comp_path)


if __name__ == '__main__':
    test_mpi_basic()
    test_mpi_categories()
    test_mpi_sub_container()
    test_mpi_all_gather()
    test_mpi_encoder()
