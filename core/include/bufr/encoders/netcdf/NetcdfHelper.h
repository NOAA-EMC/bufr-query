/*
* (C) Copyright 2024 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#pragma once

#include <string>
#include <netcdf>
#include <ostream>

#include "eckit/exception/Exceptions.h"

namespace nc = netCDF;


namespace bufr {
namespace encoders {
namespace netcdf {

    template<typename T>
    inline nc::NcType getNcType()
    {
        static_assert(!std::is_same<T, T>::value, "Unsupported type for NetCDF.");
    }

    template<> inline nc::NcType getNcType<float>() { return nc::NcType::nc_FLOAT; }
    template<> inline nc::NcType getNcType<double>() { return nc::NcType::nc_DOUBLE; }
    template<> inline nc::NcType getNcType<uint32_t>() { return nc::NcType::nc_UINT; }
    template<> inline nc::NcType getNcType<uint64_t>() { return nc::NcType::nc_UINT64; }
    template<> inline nc::NcType getNcType<int32_t>() { return nc::NcType::nc_INT; }
    template<> inline nc::NcType getNcType<int64_t>() { return nc::NcType::nc_INT64; }
    template<> inline nc::NcType getNcType<std::string>() { return nc::NcType::nc_STRING; }
    template<> inline nc::NcType getNcType<char>() { return nc::NcType::nc_CHAR; }

}  // namespace netcdf
}  // namespace encoders
}  // namespace bufr
