# (C) Copyright 2024 NOAA/NWS/NCEP/EMC
import sys
import os

import bufr
from bufr.encoders import netcdf
import numpy as np


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
        assert(os.system(f'nccmp -d -m -g -f -S  {OUTPUT_PATH} {COMP_PATH}') == 0)


def test_mpi_categories():
    DATA_PATH = 'testdata/gdas.t12z.mtiasi.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_mtiasi_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_mtiasi_{splits/satId}_cats.nc'
    COMP_PATH = 'testoutput/bufrtest_mtiasi_metop-c.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        assert(os.system(f'nccmp -d -m -g -f -S testrun/bufrtest_mtiasi_metop-c_cats.nc {COMP_PATH}') == 0)


def test_mpi_sub_container():
    DATA_PATH = 'testdata/gdas.t12z.mtiasi.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_mtiasi_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_mtiasi_{splits/satId}_sub_container.nc'
    COMP_PATH = 'testoutput/bufrtest_mtiasi_metop-c.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container = container.get_sub_container(['metop-c'])
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        assert(os.system(f'nccmp -d -m -g -f -S testrun/bufrtest_mtiasi_metop-c_sub_container.nc {COMP_PATH}') == 0)


def test_mpi_all_gather():
    DATA_PATH = 'testdata/gdas.t06z.snocvr.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_long_strs_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_long_strs.nc'
    COMP_PATH = 'testoutput/bufrtest_long_strs.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.all_gather(comm)

    if comm.rank() == 1:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)
        assert(os.system(f'nccmp -d -m -g -f -S  {OUTPUT_PATH} {COMP_PATH}') == 0)


if __name__ == '__main__':
    test_mpi_basic()
    test_mpi_categories()
    test_mpi_sub_container()
    test_mpi_all_gather()
