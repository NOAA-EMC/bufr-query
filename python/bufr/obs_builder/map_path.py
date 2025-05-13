
import os

def map_path(map_file_name, base_dir=None):
    """
    Construct the absolute path to a mapping file.

    If `base_dir` is provided, the path will be relative to it.
    Otherwise, the path is resolved relative to the current working directory.

    :param map_file_name: Name of the mapping file (e.g., 'bufr_satwnd_ascat.yaml').
    :type map_file_name: str
    :param base_dir: Optional base directory to use instead of the current working directory.
    :type base_dir: str or None

    :returns: Absolute path to the mapping file.
    :rtype: str

    :raises FileNotFoundError: If the resolved file path does not exist.
    """
    root_dir = base_dir if base_dir is not None else os.getcwd()
    path = os.path.join(root_dir, map_file_name)

    if not os.path.exists(path):
        raise FileNotFoundError(f"Mapping file not found: {path}")

    return path
