// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "bufr/File.h"

#include <algorithm>

#include "QueryRunner.h"
#include "bufr/QuerySet.h"
#include "bufr/DataProvider.h"
#include "bufr/NcepDataProvider.h"
#include "bufr/WmoDataProvider.h"


namespace bufr {
    File::File(const std::string &filename, const std::string &wmoTablePath)
    {
        if (wmoTablePath.empty())
        {
            dataProvider_ = std::make_shared<NcepDataProvider>(filename);
        }
        else
        {
            dataProvider_ = std::make_shared<WmoDataProvider>(filename, wmoTablePath);
        }

        dataProvider_->open();
    }

    size_t File::size(const QuerySet& querySet, const RunParameters& params)
    {
        return dataProvider_->numMessages(querySet, params);
    }

    void File::close()
    {
        dataProvider_->close();
    }

    void File::rewind()
    {
        dataProvider_->rewind();
    }

    ResultSet File::execute(const QuerySet &querySet, const RunParameters& params)
    {
        auto resultSet = ResultSet();
        auto queryRunner = QueryRunner(querySet, resultSet, dataProvider_);

        auto processMsg = [] () mutable
        {
        };

        auto processSubset = [&queryRunner]() mutable
        {
            queryRunner.accumulate();
        };

        auto continueProcessing = []() -> bool
        {
          return true;
        };

        dataProvider_->run(querySet,
                           processSubset,
                           processMsg,
                           continueProcessing,
                           params);

        return resultSet;
    }

}  // namespace bufr
