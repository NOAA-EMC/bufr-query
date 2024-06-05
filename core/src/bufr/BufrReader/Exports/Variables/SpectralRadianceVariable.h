// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "eckit/config/LocalConfiguration.h"

#include "bufr/Variable.h"


namespace bufr {
    /// \brief Exports parsed data as SpectralRadiance  using speciefied Mnemonics
    class SpectralRadianceVariable final : public Variable
    {
     public:
        SpectralRadianceVariable() = delete;
        SpectralRadianceVariable(const std::string& exportName,
                                 const std::string& groupByField,
                                 const eckit::LocalConfiguration& conf);

        ~SpectralRadianceVariable() final = default;

        /// \brief Get the configured mnemonics and turn them into SpectralRadiance
        /// \param map BufrDataMap that contains the parsed data for each mnemonic
        std::shared_ptr<DataObjectBase> exportData(const BufrDataMap& map) final;

        /// \brief Get a list of queries for this variable
        QueryList makeQueryList() const final;

     private:
        /// \brief makes sure the bufr data map has all the required keys.
        void checkKeys(const BufrDataMap& map);

        /// \brief get the export key string
        std::string getExportKey(const std::string& name) const;
    };
}  // namespace bufr
