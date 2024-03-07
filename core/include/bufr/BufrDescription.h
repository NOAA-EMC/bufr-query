/*
 * (C) Copyright 2020-2024 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "Export.h"

namespace bufr {

    /// \brief Description of the data to be read from a BUFR file and how to expose that data to
    /// the outside world.
    class BufrDescription
    {
     public:
        explicit BufrDescription(const std::string& yamlPath);
        explicit BufrDescription(const eckit::Configuration &conf);

        // Setters
        /// \brief Set the relative path to the master tables (applies to wmo BUFR files).
        inline void setTablepath(const std::string& tablepath) { tablepath_ = tablepath; }

        // Getters
        /// \brief Returns the relative path to the master tables (applies to wmo BUFR files).
        inline std::string tablepath() const { return tablepath_; }

        /// \brief Returns the Export description for the BUFR file.
        inline Export getExport() const { return export_; }

     private:
       /// \brief Specifies the relative path to the master tables (applies to std BUFR files).
       std::string tablepath_;

        /// \brief Map of export strings to Variable classes.
        Export export_;
    };
}  // namespace bufr
