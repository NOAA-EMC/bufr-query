import numpy as np


def compute_wind_components(speed, direction, in_degrees=True):
    """
    Convert wind speed and direction to u and v wind components.

    :param speed: Wind speed in meters per second (m/s).
    :type speed: float or array-like
    :param direction: Wind direction (from which the wind is blowing), in degrees or radians.
    :type direction: float or array-like
    :param in_degrees: If True, `direction` is in degrees. If False, in radians.
    :type in_degrees: bool

    :returns: 
        - u (float or ndarray): Zonal wind component (positive is eastward).
        - v (float or ndarray): Meridional wind component (positive is northward).
    :rtype: tuple of float or tuple of ndarray

    :raises ValueError: If inputs are not broadcastable to the same shape.

    .. note::
       This function follows the meteorological convention where wind direction is
       the direction **from which** the wind is blowing.
    """
    angle = np.deg2rad(direction) if in_degrees else direction
    u = -speed * np.sin(angle)
    v = -speed * np.cos(angle)

    return u.astype(np.float32), v.astype(np.float32)
