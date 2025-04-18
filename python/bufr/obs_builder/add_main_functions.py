
import os
import inspect
import functools

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
        # Call the original method on an instance of cls
        if 'config' in inspect.signature(cls.__init__).parameters:
            kwargs.pop('config', None)
            return getattr(cls(config=kwargs['config']), method_name)(*args, **kwargs)
        else:
            return getattr(cls(), method_name)(*args, **kwargs)

    # Use functools.wraps to copy name, docstring, etc., from the original method
    wrapper = functools.wraps(method)(wrapper)
    # Assign the exact signature to the wrapper so it appears correct to inspect and help
    wrapper.__signature__ = new_sig
    return wrapper

def add_main_functions(cls):
    create_obs_group = _create_module_func(cls, 'create_obs_group')
    create_obs_file = _create_module_func(cls, 'create_obs_file')

    def make_obs_builder(config:dict=None):
        if 'config' in inspect.signature(cls.__init__).parameters:
            return cls(config=config) if config else cls()
        else:
            return cls()

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

        parser.add_argument(f'--config', required=False)
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

            create_args = {k: v for k, v in vars(args).items() if k in create_file_sig.parameters}
            create_kwargs = {k: v for k, v in vars(args).items() if k not in create_file_sig.parameters}
            create_kwargs.pop('config')

            create_obs_file(**create_args, **create_kwargs)

        end_time = time.time()
        running_time = end_time - start_time
        logger.info(f'Total running time: {running_time}')

    caller_frame = inspect.stack()[1]
    calling_module = inspect.getmodule(caller_frame.frame)
    calling_module.make_obs_builder = make_obs_builder
    calling_module.create_obs_group = create_obs_group
    calling_module.create_obs_file = create_obs_file
    calling_module.create_obs_file_from_config = create_obs_file_from_config
    calling_module.default_main = default_main

    if calling_module.__name__ == '__main__':
        default_main()
