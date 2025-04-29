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

    def create_obs_file(self, input1, input2, output):
        # Custom implementation for testing
        pass

    def create_obs_group(self, input1, input2):
        # Custom implementation for testing
        pass

add_main_functions(TestObsBuilder, execute_main=False)

def test_obs_builder_interface():
    module_functions = [member[1] for member in inspect.getmembers(__import__(__name__), inspect.isfunction)]

    function_names = [func.__name__ for func in module_functions]

    assert 'create_obs_file' in function_names, "create_obs_file function not found"
    # assert 'create_obs_group' in function_names, "create_obs_group function not found"
    assert 'default_main' in function_names, "default_main function not found"

    # check the signature of the create_obs_file function
    create_obs_file_sig = inspect.signature(create_obs_file)
    assert 'input1' in create_obs_file_sig.parameters, "input1 parameter not found in create_obs_file signature"
    assert 'input2' in create_obs_file_sig.parameters, "input2 parameter not found in create_obs_file signature"
    assert 'output' in create_obs_file_sig.parameters, "output parameter not found in create_obs_file signature"
    assert 'config' in create_obs_file_sig.parameters, "config parameter not found in create_obs_file signature"

    # check the signature of the create_obs_group function
    create_obs_group_sig = inspect.signature(create_obs_group)
    assert 'input1' in create_obs_group_sig.parameters, "input1 parameter not found in create_obs_group signature"
    assert 'input2' in create_obs_group_sig.parameters, "input2 parameter not found in create_obs_group signature"
    assert 'config' in create_obs_group_sig.parameters, "config parameter not found in create_obs_group signature"


if __name__ == '__main__':
    test_obs_builder_interface()
