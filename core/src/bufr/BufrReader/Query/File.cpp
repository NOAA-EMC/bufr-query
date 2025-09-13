// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "bufr/File.h"

#include <algorithm>

#include "QueryRunner.h"
#include "bufr/QuerySet.h"
#include "bufr/DataProvider.h"
#include "bufr/NcepDataProvider.h"
#include "bufr/WmoDataProvider.h"
#include "Log.h"


namespace bufr {
    File(const std::string& filename,
         const std::map<std::string, int>& bufrParams)
    {
        dataProvider_ = std::make_shared<NcepDataProvider>(filename);

        for (const auto& param : bufrParams)
        {
            if (!dataProvider_->setParam(param.first, param.second))
            {
               log::warning << "Failed to set BUFR param " << param.first << " to " << param.second;
            }
        }

        dataProvider_->open();
    }

    File::File(const std::string &filename,
               const std::string &wmoTablePath,
               const std::map<std::string, int>& bufrParams)
    {
        dataProvider_ = std::make_shared<WmoDataProvider>(filename, wmoTablePath);

        for (const auto& param : bufrParams)
        {
            if (!dataProvider_->setParam(param.first, param.second))
            {
                log::warning << "Failed to set BUFR param " << param.first << " to " << param.second;
            }
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

        auto continueProcessing = [numMessages, &msgCnt, offset]() -> bool
        {
            if (numMessages > 0 && msgCnt > offset)
            {
              return  (msgCnt - offset) < numMessages;
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
