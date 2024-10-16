// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include <memory>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "Filter.h"
#include "Split.h"
#include "Variable.h"


namespace bufr {
    /// \brief Uses configuration to determine all the things needed to be done on export.
    class Export
    {
     public:
        typedef std::vector<std::shared_ptr<Split>> Splits;
        typedef std::vector<std::shared_ptr<Variable>> Variables;
        typedef std::vector<std::shared_ptr<Filter>> Filters;

        /// \brief Constructor
        Export() = default;

        /// \brief Constructor
        /// \param conf Config data/
        explicit Export(const eckit::Configuration &conf);

        // Getters
        inline Splits getSplits() const { return splits_; }
        inline Variables getVariables() const { return variables_; }
        inline Filters getFilters() const { return filters_; }
        inline std::vector<std::string> getSubsets() const { return subsets_; }

     private:
        Splits splits_;
        Variables  variables_;
        Filters filters_;
        std::vector<std::string> subsets_;


        /// \brief Create Variables exports from config.
        void addVariables(const eckit::Configuration &conf,
                          const std::string& groupByVariable = "");

        /// \brief Create Splits exports from config.
        void addSplits(const eckit::Configuration &conf);

        /// \brief Create Filters exports from config.
        void addFilters(const eckit::Configuration &conf);
    };
}  // namespace bufr
