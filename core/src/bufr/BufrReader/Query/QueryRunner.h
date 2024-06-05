// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "bufr/DataProvider.h"
#include "bufr/SubsetVariant.h"
#include "bufr/QuerySet.h"
#include "bufr/ResultSet.h"
#include "Target.h"

namespace bufr {
    /// \brief Manages the execution of queries against on a BUFR file.
    class QueryRunner
    {
     public:
        /// \brief Constructor.
        /// \param[in] querySet The set of queries to execute against the BUFR file.
        /// \param[in, out] resultSet The object used to store the accumulated collected data.
        /// \param[in] dataProvider The BUFR data provider to use.
        QueryRunner(const QuerySet& querySet, ResultSet& resultSet,
                    const DataProviderType& dataProvider);

        /// \brief Run the queries against the currently open BUFR message subset. Collect the
        /// results into the ResultSet.
        void accumulate();

     private:
        const QuerySet querySet_;
        ResultSet& resultSet_;
        const DataProviderType& dataProvider_;

        std::unordered_map<SubsetVariant, std::shared_ptr<Targets>> targetsCache_;

        /// \brief Look for the list of targets for the currently active BUFR message subset that
        /// apply to the QuerySet and cache them.
        /// \param[in, out] targets The list of targets to populate.
        std::shared_ptr<Targets> getTargets();
    };
}  // namespace bufr
