// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include "bufr/Filter.h"

#include <memory>
#include <string>
#include <vector>

namespace bufr {
    /// \brief Class that filter data given optional upper and lower bounds.
    class BoundingFilter : public Filter
    {
     public:
        /// \brief Constructor
        /// \param conf The configuration for this filter
        explicit BoundingFilter(const eckit::LocalConfiguration& conf);

        virtual ~BoundingFilter() = default;

        /// \brief Apply the filter to the dataevant data.
        void apply(BufrDataMap& dataMap) final;

        /// \param dataMap Map to modify by filtering out rel
     private:
         const std::string variable_;
         std::shared_ptr<float> lowerBound_;
         std::shared_ptr<float> upperBound_;
    };
}  // namespace bufr
