// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "bufr/BufrTypes.h"

namespace bufr {
    /// \brief Base class for all Split objects that split data into sub-parts
    class Split
    {
     public:
        Split(const std::string& name, const eckit::LocalConfiguration& conf)  :
            name_(name),
            conf_(conf) {}

        virtual ~Split() = default;

        /// \brief Get set of sub categories this split will create
        /// \param dataMap The data we will split on.
        /// \result set of unique strings
        virtual std::vector<std::string> subCategories(const BufrDataMap& dataMap) = 0;

        /// \brief Split the data according to internal rules
        /// \param dataMap Data to be split
        /// \result map of split data where the category is the key
        virtual std::unordered_map<std::string, BufrDataMap> split(const BufrDataMap& dataMap) = 0;

        /// \brief Get the split name
        inline std::string getName() const { return name_; }

     protected:
        /// \brief The name of the split as defined by the key in the YAML file.
        const std::string name_;

        /// \brief Configuration associated with this split
        const eckit::LocalConfiguration conf_;
    };
}  // namespace bufr
