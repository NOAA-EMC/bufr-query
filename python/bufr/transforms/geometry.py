import os
import pytz
from pysolar.solar import get_altitude, get_azimuth
from datetime import datetime, timezone

def compute_solar_angles(lat, lon, unix_time):
    """
    Compute solar zenith and azimuth angles for a single point.

    :param lat: Latitude in degrees.
    :type lat: float
    :param lon: Longitude in degrees.
    :type lon: float
    :param unix_time: Unix timestamp (seconds since 1970-01-01T00:00:00Z).
    :type unix_time: int

    :return: A tuple containing zenith angle and azimuth angle.
    :rtype: tuple(float, float)
    """

    dt = datetime.fromtimestamp(int(unix_time), tz=timezone.utc)
    altitude = get_altitude(lat, lon, dt)
    azimuth = get_azimuth(lat, lon, dt)
    zenith = 90.0 - altitude

    return zenith, azimuth

