import numpy as np

def add_dummy_variable(container, name, cat, base_var):
    """
    Add a dummy variable to the observation container using a zero-length masked array.

    This function is typically used when a sub-category exists but contains no valid data,
    ensuring the variable still exists in the container with the correct structure.

    Parameters
    ----------
    container : object
        The observation data container. Must support `get_paths(base_var, cat)` and `add(name, data, paths, cat)`.

    name : str
        The name of the variable to add to the container.

    cat : tuple
        The sub-category identifier (usually a tuple of integers or strings).

    base_var : str
        The name of an existing variable in the container to base the path and dtype on.

    Raises
    ------
    KeyError
        If `base_var` is not found or paths cannot be resolved.

    Returns
    -------
    None
    """
    try:
        paths = container.get_paths(base_var, cat)
        dummy = container.get(base_var, cat)
        container.add(name, dummy, paths, cat)

    except Exception as e:
        raise KeyError(f"Failed to add dummy variable '{name}' using base '{base_var}' in category {cat}: {e}")
