// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

/** @file
    @brief Define signature to enable the spatial average module
    written in Fortran 90 to be called via wrapper functions from C and C++
    application programs.

 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  void Spatial_Average_f(int bufrsat, int method, int num_obs, int nchanl, float missing, void* fov, void* rainflag, 
                         void* time, void* lat, void* lon, void* bt_inout,
                         int* error_status);

#ifdef __cplusplus
}
#endif


