// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "QueryVariable.h"

#include <ostream>

#include "Transforms/TransformBuilder.h"
#include "eckit/exception/Exceptions.h"

namespace
{
    namespace ConfKeys
    {
        const char *Query = "query";
        const char *Type = "type";
    }
}


namespace bufr {
    QueryVariable::QueryVariable(const std::string& exportName,
                                 const std::string& groupByField,
                                 const eckit::LocalConfiguration& conf) :
        Variable(exportName, groupByField, conf)
    {
        initQueryMap();
    }

    std::shared_ptr<DataObjectBase> QueryVariable::exportData(const BufrDataMap& map)
    {
        if (map.find(getExportName()) == map.end())
        {
            std::stringstream errStr;
            errStr << "Export named " << getExportName();
            errStr << " could not be found during export.";
            throw eckit::BadParameter(errStr.str());
        }

        auto dataObject = map.at(getExportName());

        for (const auto& transform : TransformBuilder::makeTransforms(conf_))
        {
            transform->apply(dataObject);
        }

        return dataObject;
    }

    QueryList QueryVariable::makeQueryList() const
    {
        auto queries = QueryList();

        QueryInfo info;
        info.name = getExportName();
        info.query = conf_.getString(ConfKeys::Query);
        info.groupByField = groupByField_;

        if (conf_.has(ConfKeys::Type))
        {
            info.type = conf_.getString(ConfKeys::Type);
        }

        queries.push_back(info);

        return queries;
    }
}  // namespace bufr
