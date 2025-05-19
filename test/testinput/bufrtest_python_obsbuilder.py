import os
import sys
import inspect
import subprocess

import bufr
from bufr.obs_builder import ObsBuilder, add_main_functions

def run_compare(input_path, comp_path):
    result = subprocess.Popen(f'nccmp -d -m -g -f -S {input_path} {comp_path}', shell=True).wait()
    assert result == 0, f"Comparison failed for {input_path} and {comp_path}."

def map_path(map_file_name):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, map_file_name)

MAPPING_PATH = map_path('bufrtest_mhs_basic_mapping.yaml')

class TestObsBuilder(ObsBuilder):
    def __init__(self):
        super().__init__(MAPPING_PATH)

    def make_obs(self, comm, input):
        # Custom implementation for testing
        return super().make_obs(comm, input)

add_main_functions(TestObsBuilder, execute_main=False)

def test_basic_obs_builder_interface():
    module_functions = [member[1] for member in inspect.getmembers(__import__(__name__), inspect.isfunction)]

    function_names = [func.__name__ for func in module_functions]

    assert 'create_obs_file' in function_names, "create_obs_file function not found"
    assert 'create_obs_group' in function_names, "create_obs_group function not found"
    assert 'default_main' in function_names, "default_main function not found"

def test_run_obs_builder():
    # Test the ObsBuilder functionality
    obs_builder = TestObsBuilder()

    # Assuming you have a valid MPI communicator and input data
    bufr.mpi.App(sys.argv) # Don't do this if passing in MPI communicator
    comm = bufr.mpi.Comm("world")

    input_data = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'

    container = obs_builder.make_obs(comm, input_data)

    # Perform assertions on the container
    assert isinstance(container, bufr.DataContainer), "Container is not of type DataContainer"


def test_run_obs_file():
    # Test the ObsBuilder file creation
    obs_builder = TestObsBuilder()

    input_path = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'
    output_path = 'testrun/bufrtest_mhs_basic.nc'
    compare_path = 'testoutput/bufrtest_mhs_basic.nc'

    create_obs_file(input_path, output_path)

    # Compare the file to the expected output
    run_compare(compare_path, output_path)

def test_run_obs_group():
    # Test the ObsBuilder group creation
    obs_builder = TestObsBuilder()

    input_path = 'testdata/gdas.t18z.1bmhs.tm00.bufr_d'
    output_path = 'testrun/bufrtest_mhs_basic.nc'
    compare_path = 'testoutput/bufrtest_mhs_basic.nc'

    env = {'comm_name':'world'}
    obs_group = create_obs_group(input_path, env)

    assert(len(obs_group.list()) > 0, "Group is empty")


if __name__ == '__main__':
    test_basic_obs_builder_interface()
    test_run_obs_builder()
    test_run_obs_file()
    test_run_obs_group()