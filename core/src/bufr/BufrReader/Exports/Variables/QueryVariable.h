// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "bufr/BufrTypes.h"
#include "bufr/DataObject.h"
#include "Transforms/Transform.h"
#include "bufr/Variable.h"

namespace bufr {
    /// \brief Exports parsed data associated with a mnemonic (ex: "CLAT")
    class QueryVariable final : public Variable
    {
     public:
        QueryVariable() = delete;
        explicit QueryVariable(const std::string& exportName,
                               const std::string& groupByField,
                               const eckit::LocalConfiguration& conf);

        ~QueryVariable() final = default;

        /// \brief Gets the requested data, applies transforms, and returns the requested data
        /// \param map BufrDataMap that contains the parsed data for each mnemonic
        std::shared_ptr<DataObjectBase> exportData(const BufrDataMap& map) final;

        /// \brief Get a list of queries for this variable
        QueryList makeQueryList() const final;
    };
}  // namespace bufr
