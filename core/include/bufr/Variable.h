// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "BufrTypes.h"
#include "DataObject.h"

namespace bufr {
    struct QueryInfo
    {
        std::string name;
        std::string query;
        std::string groupByField;
        std::string type;
    };

    typedef std::string QueryName;
    typedef std::vector<QueryInfo> QueryList;

    /// \brief Abstract base class for all Exports.
    class Variable
    {
     public:
        Variable() = delete;

        explicit Variable(const std::string& exportName,
                          const std::string& groupByField,
                          const eckit::LocalConfiguration& conf) :
            groupByField_(groupByField),
            conf_(conf),
            exportName_(exportName)
        {}

        virtual ~Variable() = default;

        /// \brief Variable data objects for previously parsed data from BufrDataMap.
        virtual std::shared_ptr<DataObjectBase> exportData(const BufrDataMap& dataMap) = 0;

        /// \brief Get Query List
        inline QueryList getQueryList() { return queryList_; }

        /// \brief Get Export Name
        inline std::string getExportName() const { return exportName_; }

     protected:
        /// \brief The for field of interest
        const std::string groupByField_;

        /// \brief The configuration object for this variable
        const eckit::LocalConfiguration conf_;

        /// \brief Initialize the query map
        inline void initQueryMap() { queryList_ = makeQueryList(); }

        /// \brief Make a map of name and queries
        virtual QueryList makeQueryList() const = 0;

     private:
        /// \brief Name used to export this variable
        std::string exportName_;

        /// \brief The query map for all the queries this variable needs
        QueryList queryList_;
    };
}  // namespace bufr


