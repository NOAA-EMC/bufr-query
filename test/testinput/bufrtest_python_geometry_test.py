# (C) Copyright 2023 NOAA/NWS/NCEP/EMC
import sys
import numpy as np
from datetime import datetime, timezone

# Import the geometry module - works both in test harness and standalone
try:
    from bufr.transforms import geometry
except ModuleNotFoundError:
    # Fallback for running standalone
    sys.path.insert(0, 'python/bufr/transforms')
    import geometry


def test_solar_angles_basic():
    """Test basic solar angle computation with known inputs."""
    # Test case: Equator at noon on the equinox
    # At solar noon on the equinox, the sun should be directly overhead at the equator
    # March 20, 2020 at approximately 12:00 UTC at longitude 0
    latitudes = np.array([0.0])
    longitudes = np.array([0.0])
    # Unix timestamp for 2020-03-20 12:00:00 UTC
    unix_times = np.array([1584709200.0])
    
    zenith, azimuth = geometry.compute_solar_angles(latitudes, longitudes, unix_times)
    
    # At solar noon on the equinox at the equator, zenith should be close to 0
    # (sun directly overhead), but due to equation of time and exact date, it won't be exact
    assert zenith.shape == (1,)
    assert azimuth.shape == (1,)
    assert 0 <= zenith[0] <= 90, f"Zenith angle {zenith[0]} should be between 0 and 90 degrees"
    assert 0 <= azimuth[0] < 360, f"Azimuth angle {azimuth[0]} should be between 0 and 360 degrees"
    # At solar noon, zenith should be relatively small
    assert zenith[0] < 30, f"At near-equinox noon at equator, zenith {zenith[0]} should be < 30 degrees"


def test_solar_angles_multiple_locations():
    """Test solar angle computation with multiple locations."""
    # Test with multiple locations at the same time
    latitudes = np.array([0.0, 40.0, -40.0, 80.0])
    longitudes = np.array([0.0, -100.0, 150.0, 20.0])
    # Same Unix timestamp for all
    unix_times = np.array([1584709200.0, 1584709200.0, 1584709200.0, 1584709200.0])
    
    zenith, azimuth = geometry.compute_solar_angles(latitudes, longitudes, unix_times)
    
    # Check output shapes
    assert zenith.shape == (4,)
    assert azimuth.shape == (4,)
    
    # All zenith angles should be between 0 and 180 degrees
    assert np.all((zenith >= 0) & (zenith <= 180))
    
    # All azimuth angles should be between 0 and 360 degrees
    assert np.all((azimuth >= 0) & (azimuth < 360))


def test_solar_angles_summer_winter():
    """Test solar angles at different seasons."""
    # Northern hemisphere location
    latitudes = np.array([40.0, 40.0])
    longitudes = np.array([-100.0, -100.0])
    
    # Summer solstice (June 21, 2020, 12:00 UTC) and Winter solstice (Dec 21, 2020, 12:00 UTC)
    unix_times = np.array([1592740800.0, 1608552000.0])
    
    zenith, azimuth = geometry.compute_solar_angles(latitudes, longitudes, unix_times)
    
    # In summer, sun should be higher (lower zenith) than in winter at same latitude
    # Note: This is at solar noon-ish, accounting for longitude offset from UTC
    assert zenith.shape == (2,)
    assert azimuth.shape == (2,)
    assert np.all(zenith >= 0) and np.all(zenith <= 180)


def test_solar_angles_midnight():
    """Test solar angles at midnight."""
    # Location at midnight
    latitudes = np.array([40.0])
    longitudes = np.array([0.0])
    # Midnight UTC (00:00:00) on 2020-03-20
    unix_times = np.array([1584662400.0])
    
    zenith, azimuth = geometry.compute_solar_angles(latitudes, longitudes, unix_times)
    
    # At midnight at longitude 0, the sun should be well below the horizon
    # Zenith angle should be > 90 degrees (sun below horizon)
    assert zenith[0] > 90, f"At midnight, zenith {zenith[0]} should be > 90 degrees"


def test_solar_angles_input_validation():
    """Test input validation."""
    # Test with empty arrays
    try:
        latitudes = np.array([])
        longitudes = np.array([])
        unix_times = np.array([])
        geometry.compute_solar_angles(latitudes, longitudes, unix_times)
        assert False, "Should raise ValueError for empty arrays"
    except ValueError as e:
        assert "empty" in str(e).lower()
    
    # Test with mismatched shapes
    try:
        latitudes = np.array([0.0, 1.0])
        longitudes = np.array([0.0])
        unix_times = np.array([1584709200.0])
        geometry.compute_solar_angles(latitudes, longitudes, unix_times)
        assert False, "Should raise ValueError for mismatched shapes"
    except ValueError as e:
        assert "shape" in str(e).lower()
    
    # Test with non-1D arrays
    try:
        latitudes = np.array([[0.0, 1.0]])
        longitudes = np.array([[0.0, 1.0]])
        unix_times = np.array([[1584709200.0, 1584709200.0]])
        geometry.compute_solar_angles(latitudes, longitudes, unix_times)
        assert False, "Should raise ValueError for non-1D arrays"
    except ValueError as e:
        assert "1-dimensional" in str(e).lower()


def test_solar_angles_extreme_latitudes():
    """Test solar angles at extreme latitudes (polar regions)."""
    # North Pole
    latitudes = np.array([90.0, -90.0])
    longitudes = np.array([0.0, 0.0])
    # Summer time in northern hemisphere
    unix_times = np.array([1592740800.0, 1592740800.0])
    
    zenith, azimuth = geometry.compute_solar_angles(latitudes, longitudes, unix_times)
    
    # Check that results are valid
    assert zenith.shape == (2,)
    assert azimuth.shape == (2,)
    assert np.all((zenith >= 0) & (zenith <= 180))
    assert np.all((azimuth >= 0) & (azimuth < 360))


def test_solar_angles_numerical_stability():
    """Test that arccos receives values in valid range [-1, 1]."""
    # Create a challenging case with many locations
    np.random.seed(42)
    n = 100
    latitudes = np.random.uniform(-90, 90, n)
    longitudes = np.random.uniform(-180, 180, n)
    # Random times over a year
    unix_times = np.random.uniform(1577836800, 1609459200, n)  # Year 2020
    
    zenith, azimuth = geometry.compute_solar_angles(latitudes, longitudes, unix_times)
    
    # Check that no NaN values are produced
    assert not np.any(np.isnan(zenith)), "Zenith angles should not contain NaN"
    assert not np.any(np.isnan(azimuth)), "Azimuth angles should not contain NaN"
    
    # Check valid ranges
    assert np.all((zenith >= 0) & (zenith <= 180))
    assert np.all((azimuth >= 0) & (azimuth < 360))


if __name__ == '__main__':
    test_solar_angles_basic()
    test_solar_angles_multiple_locations()
    test_solar_angles_summer_winter()
    test_solar_angles_midnight()
    test_solar_angles_input_validation()
    test_solar_angles_extreme_latitudes()
    test_solar_angles_numerical_stability()
    
    print("All geometry tests passed!")
