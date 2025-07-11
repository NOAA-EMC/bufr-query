
import os
import sys
import inspect
import functools
import yaml
import json
import bufr

def add_main_functions(cls, execute_main=True):

    def make_obs_builder(config:dict=None):
        if 'config' in inspect.signature(cls.__init__).parameters:
            return cls(config=config) if config else cls()
        else:
            return cls()

    def default_main():
        import time
        import yaml
        import argparse
        from bufr.obs_builder import Logger

        start_time = time.time()

        mpi.App(sys.argv)
        comm = mpi.Comm("world")

        logger = Logger(os.path.basename(__file__), comm)

        create_file_sig = inspect.signature(create_obs_file)

        # Required input arguments
        parser = argparse.ArgumentParser()

        for param in create_file_sig.parameters.values():
            if param.name == 'self':
                continue
            if param.default is param.empty:
                parser.add_argument(f'--{param.name}', required=True)
            else:
                parser.add_argument(f'--{param.name}', default=param.default)
        args = parser.parse_args()

        create_args = {k: v for k, v in vars(args).items() if k in create_file_sig.parameters}
        create_kwargs = {k: v for k, v in vars(args).items() if k not in create_file_sig.parameters}

        create_obs_file(**create_args, **create_kwargs)

        end_time = time.time()
        running_time = end_time - start_time
        logger.info_all(f'Total running time (default_main): {running_time} seconds')

    def _create_module_func(cls, method_name):
        """Create a module-level function that calls cls.method_name with the same signature."""
        # Get the bound method and its signature
        method = getattr(cls, method_name)
        sig = inspect.signature(method)

        # Remove 'self' from parameters for the new function
        params = [param for name, param in sig.parameters.items() if name != 'self']
        # Add optional 'config' parameter to the parameters
        params.append(inspect.Parameter('config',
                                        inspect.Parameter.KEYWORD_ONLY,
                                        default={}))

        new_sig = inspect.Signature(params)

        # Define a generic wrapper that calls the method on a new instance
        def wrapper(*args, **kwargs):
            config = kwargs.pop('config', {})
            if config and isinstance(config, str):
                ext = os.path.splitext(config)[-1]
                if ext == '.yaml' or ext == '.yml':
                    # Load the config from a YAML file if it's a string
                    with open(config, 'r') as f:
                        config = yaml.safe_load(f)
                # Load the config from a JSON file if it's a string
                elif ext == '.json':
                    with open(config, 'r') as f:
                        config = json.load(f)
                else:
                    raise ValueError(f'Config file must be a .yaml or .json file.')

            if not isinstance(config, dict):
                raise ValueError(f'Config must resolve to a dict.')

            bufr.mpi.App(sys.argv)
            return getattr(make_obs_builder(config), method_name)(*args, **kwargs)

        # Use functools.wraps to copy name, docstring, etc., from the original method
        wrapper = functools.wraps(method)(wrapper)
        # Assign the exact signature to the wrapper so it appears correct to inspect and help
        wrapper.__signature__ = new_sig
        return wrapper

    create_obs_group = _create_module_func(cls, 'create_obs_group')
    create_obs_file = _create_module_func(cls, 'create_obs_file')

    caller_frame = inspect.stack()[1]
    calling_module = inspect.getmodule(caller_frame.frame)
    calling_module.make_obs_builder = make_obs_builder
    calling_module.create_obs_group = create_obs_group
    calling_module.create_obs_file = create_obs_file
    calling_module.default_main = default_main

    if calling_module.__name__ == '__main__' and execute_main:
        default_main()
