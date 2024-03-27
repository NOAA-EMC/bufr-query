//
// Created by Ronald McLaren on 3/25/24.
//

#pragma once

#include <string>
#include <netcdf>

#include "eckit/exception/Exceptions.h"

namespace nc = netCDF;


namespace bufr {
namespace encoders {
namespace netcdf {

    template<typename T>
    nc::NcType getNcType()
    {
        if (std::is_same<T, float>::value)
        {
            return nc::NcType::nc_FLOAT;
        }
        else if (std::is_same<T, double>::value)
        {
            return nc::NcType::nc_DOUBLE;
        }
        else if (std::is_same<T, uint32_t>::value)
        {
            return nc::NcType::nc_UINT;
        }
        else if (std::is_same<T, uint64_t>::value)
        {
            return nc::NcType::nc_UINT64;
        }
        else if (std::is_same<T, long long>::value)
        {
            return nc::NcType::nc_INT64;
        }
        else if (std::is_same<T, int32_t>::value)
        {
            return nc::NcType::nc_INT;
        }
        else if (std::is_same<T, int64_t>::value)
        {
            return nc::NcType::nc_INT64;
        }
        else if (std::is_same<T, std::string>::value)
        {
            return nc::NcType::nc_CHAR;
        }
        else
        {
            throw eckit::BadParameter("Unsupported type encountered for NetCDF encoding.");
        }
    }
}
}
}
