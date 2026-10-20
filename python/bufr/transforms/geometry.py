import numpy as np
from datetime import datetime, timezone
from typing import Tuple

# Constants
DEG_TO_RAD = np.pi / 180.0
r60inv = 1.0 / 60.0

# Analemma data
nday = np.array(
        [ 1.0, 6.0, 11.0, 16.0, 21.0, 26.0, 31.0, 36.0, 41.0, 46.0,
         51.0, 56.0, 61.0, 66.0, 71.0, 76.0, 81.0, 86.0, 91.0, 96.0,
         101.0, 106.0, 111.0, 116.0, 121.0, 126.0, 131.0, 136.0, 141.0, 146.0,
         151.0, 156.0, 161.0, 166.0, 171.0, 176.0, 181.0, 186.0, 191.0, 196.0,
         201.0, 206.0, 211.0, 216.0, 221.0, 226.0, 231.0, 236.0, 241.0, 246.0,
         251.0, 256.0, 261.0, 266.0, 271.0, 276.0, 281.0, 286.0, 291.0, 296.0,
         301.0, 306.0, 311.0, 316.0, 321.0, 326.0, 331.0, 336.0, 341.0, 346.0,
         351.0, 356.0, 361.0, 366.0 ], dtype=np.float64)
eqt = np.array(
        [ -3.23, -5.49, -7.60, -9.48, -11.09, -12.39, -13.34, -13.95, -14.23, -14.19,
          -13.85, -13.22, -12.35, -11.26, -10.01, -8.64, -7.18, -5.67, -4.16, -2.69,
          -1.29, -0.02, 1.10, 2.05, 2.80, 3.33, 3.63, 3.68, 3.49, 3.09,
           2.48, 1.71, 0.79, -0.24, -1.33, -2.41, -3.45, -4.39, -5.20, -5.84,
          -6.28, -6.49, -6.44, -6.15, -5.60, -4.82, -3.81, -2.60, -1.19, 0.36,
           2.03, 3.76, 5.54, 7.31, 9.04, 10.69, 12.20, 13.53, 14.65, 15.52,
           16.12, 16.41, 16.36, 15.95, 15.19, 14.09, 12.67, 10.93, 8.93, 6.70,
           4.32, 1.86, -0.62, -3.23 ], dtype=np.float64)
dec = np.array(
        [ -23.06, -22.57, -21.91, -21.06, -20.05, -18.88, -17.57, -16.13, -14.57, -12.91,
            -11.16, -9.34, -7.46, -5.54, -3.59, -1.62, 0.36, 2.33, 4.28, 6.19,
            8.06, 9.88, 11.62, 13.29, 14.87, 16.34, 17.70, 18.94, 20.04, 21.00,
            21.81, 22.47, 22.95, 23.28, 23.43, 23.40, 23.21, 22.85, 22.32, 21.63,
            20.79, 19.80, 18.67, 17.42, 16.05, 14.57, 13.00, 11.33, 9.60, 7.80,
            5.95, 4.06, 2.13, 0.19, -1.75, -3.69, -5.62, -7.51, -9.36, -11.16,
            -12.88, -14.53, -16.07, -17.50, -18.81, -19.98, -20.99, -21.85, -22.52, -23.02,
            -23.33, -23.44, -23.35, -23.06 ], dtype=np.float64)

def compute_solar_angles(latitudes: np.ndarray, longitudes: np.ndarray, unix_times: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
    """
    Compute solar zenith and azimuth angles following the GSI solar position algorithm (zensun).

    This function implements the solar position calculation from NOAA-EMC/GSI, which uses
    analemma data (equation of time and solar declination) interpolated over day of year
    to compute accurate solar angles.

    :param latitudes: Array of latitudes in degrees. Must be a NumPy array.
    :param longitudes: Array of longitudes in degrees. Must be a NumPy array.
    :param unix_times: Array of Unix timestamps (seconds since 1970-01-01T00:00:00Z). Must be a NumPy array.

    :return: Two arrays (zenith_angles, azimuth_angles), both in degrees.
        - zenith_angles: Solar zenith angle in degrees (0° = sun directly overhead, 90° = sun at horizon).
        - azimuth_angles: Solar azimuth angle in degrees, measured clockwise from North
          (0° = North, 90° = East, 180° = South, 270° = West), normalized to [0, 360].

    :raises ValueError: If input arrays do not have the same shape.

    .. note::
       All inputs must be NumPy arrays with the same shape. Scalar inputs are not supported.

    .. seealso::
       GSI (Gridpoint Statistical Interpolation) repository:
       https://github.com/NOAA-EMC/GSI/blob/develop/src/gsi/read_amsre.f90#L695
    """
    # Validate input arrays
    if latitudes.size == 0 or longitudes.size == 0 or unix_times.size == 0:
        raise ValueError("Input arrays must not be empty.")

    if not (latitudes.shape == longitudes.shape == unix_times.shape):
        raise ValueError("All input arrays must have the same shape.")

    if latitudes.ndim != 1 or longitudes.ndim != 1 or unix_times.ndim != 1:
        raise ValueError("Input arrays must be 1-dimensional.")

    # Convert Unix times to NumPy datetime64 and compute fractional days
    unix_times_int = unix_times.astype("int64")
    dt64 = unix_times_int.astype("datetime64[s]")
    dates = dt64.astype("datetime64[D]")
    years = dt64.astype("datetime64[Y]")
    day_of_year = (dates - years).astype("timedelta64[D]").astype(int) + 1
    seconds_since_midnight = (dt64 - dates).astype("timedelta64[s]").astype(int)
    hours_since_midnight = seconds_since_midnight / 3600.0

    # Compute fractional day number with 1 January as day 1
    fractional_day = day_of_year + hours_since_midnight / 24.0

    # Interpolate equation of time (eqt) and declination (dec)
##    eqtime = np.interp(fractional_day, nday, eqt) * r60inv  # Convert to hours
##    declination = np.interp(fractional_day, nday, dec)

    num_obs = len(unix_times_int)
    eqtime = np.zeros(num_obs, dtype=np.float32)
    declination = np.zeros(num_obs, dtype=np.float32)
    for iobs in range(num_obs):
        fractional_day0 = fractional_day[iobs]
        eqtime0,declination0 = zensun(fractional_day0)
        eqtime[iobs] = eqtime0
        declination[iobs] = declination0

    # Compute solar position angles
    hour_angle = -15.0 * (hours_since_midnight - 12 + eqtime)
    cos_zenith = (
        np.sin(latitudes * DEG_TO_RAD) * np.sin(declination * DEG_TO_RAD)
        + np.cos(latitudes * DEG_TO_RAD)
        * np.cos(declination * DEG_TO_RAD)
        * np.cos((hour_angle - longitudes) * DEG_TO_RAD)
    )
    cos_zenith = np.clip(cos_zenith, -1.0, 1.0)
    zenith_angles = np.degrees(np.arccos(cos_zenith))
    azimuth_angles = np.degrees(
        np.arctan2(
            np.cos(declination * DEG_TO_RAD) * np.sin((hour_angle - longitudes) * DEG_TO_RAD),
            np.cos(latitudes * DEG_TO_RAD) * np.sin(declination * DEG_TO_RAD) -
            np.sin(latitudes * DEG_TO_RAD) * np.cos(declination * DEG_TO_RAD) * np.cos((hour_angle - longitudes) * DEG_TO_RAD)
        )
    )
    azimuth_angles = (azimuth_angles + 360.0) % 360.0  # Normalize to [0, 360]

    return zenith_angles.astype(np.float32), azimuth_angles.astype(np.float32)


def zensun(fractional_day: float):
    """Calculates the equation of time (in hours/fractions) and solar declination angle.

    Parameters:
        fractional_day (float): Fractional day number with 12am 1 Jan = 1

    Returns:
        tuple[float, float]: (eqtime, decang)
            - eqtime: Equation of time
            - decang: Solar declination angle
    """

#    tt = ((float(day) + time / 24.0 - 1.0) % 365.25) + 1.0
    tt = fractional_day

    di = 0
    for i in range(73):
        if nday[i] <= tt <= nday[i + 1]:
            di = i
            break

    y = np.zeros(5, dtype=np.float64)
    y2 = np.zeros(5, dtype=np.float64)

    x = np.ones((2, 5), dtype=np.float64)

    # Perform linear regression setup depending on boundary conditions
    if 2 <= di <= 71:
        y[:] = eqt[di - 2 : di + 3]
        y2[:] = dec[di - 2 : di + 3]
        x[1, :] = nday[di - 2 : di + 3] ** 3

    elif di == 1:
        y[0] = eqt[72]
        y[1:5] = eqt[di - 1 : di + 3]
        y2[0] = dec[72]
        y2[1:5] = dec[di - 1 : di + 3]

        x[1, 0] = nday[72] ** 3
        x[1, 1:5] = (365.0 + nday[di - 1 : di + 3]) ** 3

    elif di == 0:
        y[0:2] = eqt[71:73]
        y[2:5] = eqt[di : di + 3]
        y2[0:2] = dec[71:73]
        y2[2:5] = dec[di : di + 3]

        x[1, 0:2] = nday[71:73] ** 3
        x[1, 2:5] = (365.0 + nday[di : di + 3]) ** 3

    elif di == 72:
        y[0:4] = eqt[di - 2 : di + 2]
        y[4] = eqt[1]
        y2[0:4] = dec[di - 2 : di + 2]
        y2[4] = dec[1]

        x[1, 0:4] = nday[di - 2 : di + 2] ** 3
        x[1, 4] = (365.0 + nday[1]) ** 3

    elif di == 73:
        y[0:3] = eqt[di - 2 : di + 1]
        y[3:5] = eqt[1:3]
        y2[0:3] = dec[di - 2 : di + 1]
        y2[3:5] = dec[1:3]

        x[1, 0:3] = nday[di - 2 : di + 1] ** 3
        x[1, 3:5] = (365.0 + nday[1:3]) ** 3

    # Matrix operations for least squares regression
    Tx = x.T  # Transpose of x (5x2)
    xTx = np.dot(x, Tx)  # (2x2)

    # Matrix Inversion
    det = xTx[0, 0] * xTx[1, 1] - xTx[0, 1] * xTx[1, 0]
    a = np.array(
        [
            [xTx[1, 1] / det, -xTx[0, 1] / det],
            [-xTx[1, 0] / det, xTx[0, 0] / det],
        ],
        dtype=np.float64,
    )

    aTx = np.dot(Tx, a)  # (5x2)

    beta = np.dot(y, aTx)  # (2,)
    beta2 = np.dot(y2, aTx)  # (2,)

    eqtime = (beta[0] + beta[1] * (tt**3)) * r60inv
    decang = beta2[0] + beta2[1] * (tt**3)

    return eqtime.astype(np.float32), decang.astype(np.float32)
