// (C) Copyright 2020 NOAA/NWS/NCEP/EMC
#pragma once

#include "eckit/config/LocalConfiguration.h"

#include "bufr/BufrTypes.h"

namespace bufr {
    /// \brief Base class for all the supported filters.
    class Filter
    {
     public:
        /// \brief Constructor
        /// \param conf The configuration for this filter
        explicit Filter(const eckit::LocalConfiguration& conf) : conf_(conf) {}

        /// \brief Apply the filter to the data
        /// \param dataMap Map to modify by filtering out relevant data.
        virtual void apply(BufrDataMap& dataMap) = 0;

     protected:
        eckit::LocalConfiguration conf_;
    };
}  // namespace bufr
