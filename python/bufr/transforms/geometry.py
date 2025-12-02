import os
import math
import numpy as np 
import pandas as pd
from datetime import datetime

def compute_solar_angles(latitudes, longitudes, unix_times):
    """
    Compute solar zenith and azimuth angles in parallel using multiprocessing.

    This is a Python port of the Fortran subroutine `zensun` from NOAA-EMC/GSI
    (src/gsi/read_amsre.f90). It reproduces the original least-squares fit over
    day-of-year used to interpolate equation-of-time and declination.

    :param latitudes: Array of latitudes in degrees.
    :type latitudes: numpy.ndarray
    :param longitudes: Array of longitudes in degrees.
    :type longitudes: numpy.ndarray
    :param unix_times: Array of Unix timestamps (seconds since 1970-01-01T00:00:00Z).
    :type unix_times: numpy.ndarray

    :return: Two arrays: zenith angles and azimuth angles.
    :rtype: tuple(numpy.ndarray, numpy.ndarray)
    """

    if not (latitudes.shape == longitudes.shape == unix_times.shape):
        raise ValueError(
            "Input arrays must all have the same shape. "
            f"Got latitudes.shape={latitudes.shape}, "
            f"longitudes.shape={longitudes.shape}, "
            f"unix_times.shape={unix_times.shape}."
        )

    deg2rad = math.pi / 180.0
    rad2deg = 180.0 / math.pi
    r60inv = 1.0 / 60.0

    # Convert unix time to hours after midnight
    time = (unix_timestamp % 86400)/3600.0 

    # Convert unix time to day of year
    datetime_object = datetime.datetime.fromtimestamp(unix_timestamp, tz=datetime.timezone.utc)
    day = datetime_object.timetuple().tm_yday

    # Analemma data (74 points) from the original Fortran code
    nday = [
        1.0, 6.0, 11.0, 16.0, 21.0, 26.0, 31.0, 36.0, 41.0, 46.0,
        51.0, 56.0, 61.0, 66.0, 71.0, 76.0, 81.0, 86.0, 91.0, 96.0,
        101.0, 106.0, 111.0, 116.0, 121.0, 126.0, 131.0, 136.0, 141.0, 146.0,
        151.0, 156.0, 161.0, 166.0, 171.0, 176.0, 181.0, 186.0, 191.0, 196.0,
        201.0, 206.0, 211.0, 216.0, 221.0, 226.0, 231.0, 236.0, 241.0, 246.0,
        251.0, 256.0, 261.0, 266.0, 271.0, 276.0, 281.0, 286.0, 291.0, 296.0,
        301.0, 306.0, 311.0, 316.0, 321.0, 326.0, 331.0, 336.0, 341.0, 346.0,
        351.0, 356.0, 361.0, 366.0
    ]

    eqt = [
        -3.23, -5.49, -7.60, -9.48, -11.09,
        -12.39, -13.34, -13.95, -14.23, -14.19,
        -13.85, -13.22, -12.35, -11.26, -10.01,
        -8.64, -7.18, -5.67, -4.16, -2.69,
        -1.29, -0.02, 1.10, 2.05, 2.80,
        3.33, 3.63, 3.68, 3.49, 3.09,
        2.48, 1.71, 0.79, -0.24, -1.33,
        -2.41, -3.45, -4.39, -5.20, -5.84,
        -6.28, -6.49, -6.44, -6.15, -5.60,
        -4.82, -3.81, -2.60, -1.19, 0.36,
        2.03, 3.76, 5.54, 7.31, 9.04,
        10.69, 12.20, 13.53, 14.65, 15.52,
        16.12, 16.41, 16.36, 15.95, 15.19,
        14.09, 12.67, 10.93, 8.93, 6.70,
        4.32, 1.86, -0.62, -3.23
    ]

    dec = [
        -23.06, -22.57, -21.91, -21.06, -20.05,
        -18.88, -17.57, -16.13, -14.57, -12.91,
        -11.16, -9.34, -7.46, -5.54, -3.59,
        -1.62, 0.36, 2.33, 4.28, 6.19,
        8.06, 9.88, 11.62, 13.29, 14.87,
        16.34, 17.70, 18.94, 20.04, 21.00,
        21.81, 22.47, 22.95, 23.28, 23.43,
        23.40, 23.21, 22.85, 22.32, 21.63,
        20.79, 19.80, 18.67, 17.42, 16.05,
        14.57, 13.00, 11.33, 9.60, 7.80,
        5.95, 4.06, 2.13, 0.19, -1.75,
        -3.69, -5.62, -7.51, -9.36, -11.16,
        -12.88, -14.53, -16.07, -17.50, -18.81,
        -19.98, -20.99, -21.85, -22.52, -23.02,
        -23.33, -23.44, -23.35, -23.06
    ]

    # Fractional day number with 12am 1 Jan = 1 (mirrors Fortran logic)
    tt = ((int(day) + time / 24.0 - 1.0) % 365.25) + 1.0

    # Find segment index di such that nday[di] <= tt <= nday[di+1], di in [0,72]
    di = 72  # default to last segment if not found (shouldn't happen)
    for i in range(73):
        if nday[i] <= tt <= nday[i + 1]:
            di = i
            break

    # Build 5-point windows with year-end wrap handling, matching Fortran cases
    y = [0.0] * 5
    y2 = [0.0] * 5
    x2 = [0.0] * 5  # second row of X (nday^3 possibly shifted by 365)

    if 2 <= di <= 71:
        # di in [3..72] 1-based
        for k in range(5):
            idx = di - 2 + k
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = nday[idx] ** 3
    elif di == 1:
        # di == 2 (1-based)
        y[0] = eqt[72]
        y2[0] = dec[72]
        for k, idx in enumerate([0, 1, 2, 3], start=1):
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = (365.0 + nday[idx]) ** 3
        x2[0] = nday[72] ** 3
    elif di == 0:
        # di == 1 (1-based)
        y[0] = eqt[71]
        y[1] = eqt[72]
        y2[0] = dec[71]
        y2[1] = dec[72]
        x2[0] = nday[71] ** 3
        x2[1] = nday[72] ** 3
        for k, idx in enumerate([0, 1, 2], start=2):
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = (365.0 + nday[idx]) ** 3
    elif di == 72:
        # di == 73 (1-based)
        for k, idx in enumerate([70, 71, 72, 73]):
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = nday[idx] ** 3
        y[4] = eqt[1]
        y2[4] = dec[1]
        x2[4] = (365.0 + nday[1]) ** 3
    elif di == 73:
        # di == 74 (1-based) -- unlikely from the search loop, but included for completeness
        for k, idx in enumerate([71, 72, 73]):
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = nday[idx] ** 3
        for k, idx in enumerate([1, 2], start=3):
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = (365.0 + nday[idx]) ** 3
    else:
        # Fallback (shouldn't happen): use a simple local window without wrap
        start = max(0, di - 2)
        end = min(len(nday) - 1, start + 5)
        idxs = list(range(start, end))
        # pad if needed
        while len(idxs) < 5:
            idxs.append(idxs[-1])
        for k, idx in enumerate(idxs):
            y[k] = eqt[idx]
            y2[k] = dec[idx]
            x2[k] = nday[idx] ** 3

    # Least squares for y ~ b0 + b1 * x2 using normal equations (2x2 system)
    # X = [1, x2]^T, so:
    s0 = 5.0
    s1 = sum(x2)
    s2 = sum(v * v for v in x2)
    det = s0 * s2 - s1 * s1
    # Inverse of [[s0, s1], [s1, s2]]
    a11 = s2 / det
    a12 = -s1 / det
    a21 = -s1 / det
    a22 = s0 / det

    # Compute beta for eqt (minutes) and beta2 for dec (degrees)
    # beta = (X^T X)^{-1} X^T y; with rows:
    # aTx[j][0] = a11*1 + a21*x2[j]; aTx[j][1] = a12*1 + a22*x2[j]
    b0 = sum(y[j] * (a11 + a21 * x2[j]) for j in range(5))
    b1 = sum(y[j] * (a12 + a22 * x2[j]) for j in range(5))
    b0_2 = sum(y2[j] * (a11 + a21 * x2[j]) for j in range(5))
    b1_2 = sum(y2[j] * (a12 + a22 * x2[j]) for j in range(5))

    eqtime = (b0 + b1 * (tt ** 3)) * r60inv  # hours
    decang = b0_2 + b1_2 * (tt ** 3)         # degrees
    latsun = decang

    ut = time
    # universal time of noon (not directly used further in original code, kept for reference)
    # noon = 12.0 - lon / 15.0

    lonsun = -15.0 * (ut - 12.0 + eqtime)

    # Spherical geometry
    t0 = (90.0 - lat) * deg2rad
    t1 = (90.0 - latsun) * deg2rad
    p0 = lon * deg2rad
    p1 = lonsun * deg2rad

    zz = math.cos(t0) * math.cos(t1) + math.sin(t0) * math.sin(t1) * math.cos(p1 - p0)
    # Clamp numerical round-off for acos
    zz = max(-1.0, min(1.0, zz))
    xx = math.sin(t1) * math.sin(p1 - p0)
    yy = math.sin(t0) * math.cos(t1) - math.cos(t0) * math.sin(t1) * math.cos(p1 - p0)

    sun_zenith = 90.0 - math.degrees(math.acos(zz))
    sun_azimuth = math.degrees(math.atan2(xx, yy))
    if sun_azimuth < 0.0:
        sun_azimuth += 360.0

    return sun_zenith.astype(np.float32), sun_azimuth.astype(np.float32)

