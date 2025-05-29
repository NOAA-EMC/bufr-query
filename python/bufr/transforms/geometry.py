import os
import numpy as np 
import pandas as pd
from pvlib.solarposition import get_solarposition
from datetime import datetime


def compute_solar_angles(latitudes, longitudes, unix_times):
    """
    Compute solar zenith and azimuth angles in parallel using multiprocessing.

    :param latitudes: Array of latitudes in degrees.
    :type latitudes: numpy.ndarray
    :param longitudes: Array of longitudes in degrees.
    :type longitudes: numpy.ndarray
    :param unix_times: Array of Unix timestamps (seconds since 1970-01-01T00:00:00Z).
    :type unix_times: numpy.ndarray
    :param nprocs: Number of processes to use. Defaults to the number of CPU cores.
    :type nprocs: int, optional

    :return: Two arrays: zenith angles and azimuth angles.
    :rtype: tuple(numpy.ndarray, numpy.ndarray)
    """

    assert len(latitudes) == len(longitudes) == len(unix_times), "Input arrays must be the same length"

    times = pd.to_datetime(unix_times, unit='s', utc=True)
    solpos = get_solarposition(times, latitudes, longitudes)
    zenith_angles = solpos['apparent_zenith'].values
    azimuth_angles = solpos['azimuth'].values

    return zenith_angles.astype(np.float32), azimuth_angles.astype(np.float32)

