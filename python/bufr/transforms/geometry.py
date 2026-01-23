import numpy as np
from datetime import datetime, timezone
from typing import Tuple

# Constants
DEG_TO_RAD = np.pi / 180.0
RAD_TO_DEG = 180.0 / np.pi
R60INV = 1.0 / 60.0

# Analemma data
NDAY = np.array([
    1.0, 6.0, 11.0, 16.0, 21.0, 26.0, 31.0, 36.0, 41.0, 46.0,
    51.0, 56.0, 61.0, 66.0, 71.0, 76.0, 81.0, 86.0, 91.0, 96.0,
    101.0, 106.0, 111.0, 116.0, 121.0, 126.0, 131.0, 136.0, 141.0, 146.0,
    151.0, 156.0, 161.0, 166.0, 171.0, 176.0, 181.0, 186.0, 191.0, 196.0,
    201.0, 206.0, 211.0, 216.0, 221.0, 226.0, 231.0, 236.0, 241.0, 246.0,
    251.0, 256.0, 261.0, 266.0, 271.0, 276.0, 281.0, 286.0, 291.0, 296.0,
    301.0, 306.0, 311.0, 316.0, 321.0, 326.0, 331.0, 336.0, 341.0, 346.0,
    351.0, 356.0, 361.0, 366.0
])
EQT = np.array([
    -3.23, -5.49, -7.60, -9.48, -11.09, -12.39, -13.34, -13.95, -14.23, -14.19,
    -13.85, -13.22, -12.35, -11.26, -10.01, -8.64, -7.18, -5.67, -4.16, -2.69,
    -1.29, -0.02, 1.10, 2.05, 2.80, 3.33, 3.63, 3.68, 3.49, 3.09,
    2.48, 1.71, 0.79, -0.24, -1.33, -2.41, -3.45, -4.39, -5.20, -5.84,
    -6.28, -6.49, -6.44, -6.15, -5.60, -4.82, -3.81, -2.60, -1.19, 0.36,
    2.03, 3.76, 5.54, 7.31, 9.04, 10.69, 12.20, 13.53, 14.65, 15.52,
    16.12, 16.41, 16.36, 15.95, 15.19, 14.09, 12.67, 10.93, 8.93, 6.70,
    4.32, 1.86, -0.62, -3.23
])
DEC = np.array([
    -23.06, -22.57, -21.91, -21.06, -20.05, -18.88, -17.57, -16.13, -14.57, -12.91,
    -11.16, -9.34, -7.46, -5.54, -3.59, -1.62, 0.36, 2.33, 4.28, 6.19,
    8.06, 9.88, 11.62, 13.29, 14.87, 16.34, 17.70, 18.94, 20.04, 21.00,
    21.81, 22.47, 22.95, 23.28, 23.43, 23.40, 23.21, 22.85, 22.32, 21.63,
    20.79, 19.80, 18.67, 17.42, 16.05, 14.57, 13.00, 11.33, 9.60, 7.80,
    5.95, 4.06, 2.13, 0.19, -1.75, -3.69, -5.62, -7.51, -9.36, -11.16,
    -12.88, -14.53, -16.07, -17.50, -18.81, -19.98, -20.99, -21.85, -22.52, -23.02,
    -23.33, -23.44, -23.35, -23.06
])

def compute_solar_angles(latitudes: np.ndarray, longitudes: np.ndarray, unix_times: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """
    Compute solar zenith and azimuth angles.

    :param latitudes: Array of latitudes in degrees.
    :param longitudes: Array of longitudes in degrees.
    :param unix_times: Array of Unix timestamps (seconds since 1970-01-01T00:00:00Z).

    :return: Two arrays (zenith angles, azimuth angles).
    """
    if not (latitudes.shape == longitudes.shape == unix_times.shape):
        raise ValueError("All input arrays must have the same shape.")

    # Convert Unix times to datetimes and fractional days
    datetimes = [datetime.fromtimestamp(ut, tz=timezone.utc) for ut in unix_times]
    day_of_year = np.array([dt.timetuple().tm_yday for dt in datetimes])
    hours_since_midnight = np.array([(dt.hour + dt.minute / 60 + dt.second / 3600) for dt in datetimes])

    # Compute fractional day number with 1 January as day 1
    fractional_day = day_of_year + hours_since_midnight / 24.0

    # Interpolate equation of time (eqt) and declination (dec)
    eqtime = np.interp(fractional_day, NDAY, EQT) * R60INV  # Convert to hours
    declination = np.interp(fractional_day, NDAY, DEC)

    # Compute solar position angles
    hour_angle = -15.0 * (hours_since_midnight - 12 + eqtime)
    zenith_angles = 90.0 - np.degrees(
        np.arccos(
            np.sin(latitudes * DEG_TO_RAD) * np.sin(declination * DEG_TO_RAD)
            + np.cos(latitudes * DEG_TO_RAD) *
            np.cos(declination * DEG_TO_RAD) * np.cos(hour_angle * DEG_TO_RAD)
        )
    )
    azimuth_angles = np.degrees(
        np.arctan2(
            np.sin(hour_angle * DEG_TO_RAD),
            np.cos(hour_angle * DEG_TO_RAD) * np.sin(latitudes * DEG_TO_RAD)
            - np.tan(declination * DEG_TO_RAD) * np.cos(latitudes * DEG_TO_RAD)
        )
    )
    azimuth_angles = (azimuth_angles + 360.0) % 360.0  # Normalize to [0, 360]

    return zenith_angles.astype(np.float32), azimuth_angles.astype(np.float32)
