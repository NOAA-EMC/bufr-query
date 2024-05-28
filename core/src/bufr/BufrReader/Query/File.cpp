/*
 * (C) Copyright 2022 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

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

    size_t File::size(const QuerySet& querySet)
    {
      return dataProvider_->numMessages(querySet);
    }

    void File::close()
    {
        dataProvider_->close();
    }

    void File::rewind()
    {
        dataProvider_->rewind();
    }

    ResultSet File::execute(const QuerySet &querySet, size_t offset, size_t numMessages)
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

        auto continueProcessing = [numMessages, &msgCnt]() -> bool
        {
            if (numMessages > 0)
            {
               return  msgCnt < numMessages;
            }

            return true;
        };

        dataProvider_->run(querySet,
                           processSubset,
                           processMsg,
                           continueProcessing,
                           offset);

        return resultSet;
    }

}  // namespace bufr
