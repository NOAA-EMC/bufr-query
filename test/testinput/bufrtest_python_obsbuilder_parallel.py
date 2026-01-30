import os
import sys
import inspect
import subprocess

import bufr
from bufr.obs_builder import ObsBuilder

def run_compare(input_path, comp_path):
    result = subprocess.Popen(f'nccmp -d -m -g -f -S {input_path} {comp_path}', shell=True).wait()
    assert result == 0, f"Comparison failed for {input_path} and {comp_path}."

def map_path(map_file_name):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, map_file_name)

MAPPING_PATH = map_path('bufrtest_mhs_mapping.yaml')


class TestObsBuilder(ObsBuilder):
    def __init__(self):
        super().__init__(MAPPING_PATH)

    def make_obs(self, comm, input):
        # Custom implementation for testing
        return super().make_obs(comm, input)


def test_mpi_encoder():
    DATA_PATH = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'
    COMP_PATH = 'testoutput/bufrtest_mhs_metop-b.nc'
    OUTPUT_PATH = 'testrun/bufrtest_mhs_{splits/satId}.nc'

    bufr.mpi.App(sys.argv)
    comm = bufr.mpi.Comm("world")
    rank = comm.rank()
    size = comm.size()

    obs_builder = TestObsBuilder()
    container = obs_builder.make_obs(comm, DATA_PATH)

    subcategories = container.all_sub_categories()
    obs_builder.log.info(f"subcategories: {subcategories}")
    obs_builder.log.info("Container with categories defined - encoding subcategories in parallel.")

    container.all_gather(comm)
    obs_builder._encode_by_rank(container, subcategories, OUTPUT_PATH, 'netcdf', False, rank, size)

    print (f"Rank {rank} finished encoding.")
    comm.barrier()

    # Only rank 0 needs to do the comparison
    if rank == 0:
        result_path ='testrun/bufrtest_mhs_metop-b.nc'
        # run_compare(result_path, COMP_PATH)


if __name__ == '__main__':
    test_mpi_encoder()
