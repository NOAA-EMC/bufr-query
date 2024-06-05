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

    void File::close()
    {
        dataProvider_->close();
    }

    void File::rewind()
    {
        dataProvider_->rewind();
    }

    ResultSet File::execute(const QuerySet &querySet, size_t next)
    {
        size_t msgCnt = 0;
        auto resultSet = ResultSet();
        auto queryRunner = QueryRunner(querySet, resultSet, dataProvider_);

        auto processMsg = [&msgCnt] () mutable
        {
            msgCnt++;
        };

        auto processSubset = [&queryRunner]() mutable
        {
            queryRunner.accumulate();
        };

        auto continueProcessing = [next, msgCnt]() -> bool
        {
            if (next > 0)
            {
               return  msgCnt < next;
            }

            return true;
        };

        dataProvider_->run(querySet,
                           processSubset,
                           processMsg,
                           continueProcessing);

        return resultSet;
    }
}  // namespace bufr
