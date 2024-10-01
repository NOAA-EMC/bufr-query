# (C) Copyright 2023 NOAA/NWS/NCEP/EMC
import sys

import bufr
from bufr.encoders import netcdf
import numpy as np


def test_mpi_basic():
    DATA_PATH = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_mhs_basic_mapping.yaml'
    OUTPUT_PATH = 'testrun/mhs_basic_parallel.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)

def test_mpi_categories():
    DATA_PATH = 'testdata/gdas.t12z.mtiasi.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_mtiasi_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_mtiasi_{splits/satId}_cats.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)

def test_mpi_sub_container():
    DATA_PATH = 'testdata/gdas.t12z.mtiasi.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_mtiasi_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_mtiasi_{splits/satId}_sub_container.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container = container.get_sub_container(['metop-b'])
    container.gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)

def test_mpi_gather_all():
    DATA_PATH = 'testdata/gdas.t06z.snocvr.tm00.bufr_d'
    YAML_PATH = 'testinput/bufrtest_long_strs_mapping.yaml'
    OUTPUT_PATH = 'testrun/bufrtest_long_strs.nc'

    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    container = bufr.Parser(DATA_PATH, YAML_PATH).parse(comm)
    container.all_gather(comm)

    if comm.rank() == 0:
        netcdf.Encoder(YAML_PATH).encode(container, OUTPUT_PATH)


if __name__ == '__main__':
    test_mpi_basic()
    test_mpi_categories()
    test_mpi_sub_container()
    test_mpi_gather_all()
