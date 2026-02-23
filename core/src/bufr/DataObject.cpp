// (C) Copyright 2024 NOAA/NWS/NCEP/EMC

#include "bufr/DataObject.h"
#include "bufr/Data.h"

namespace bufr {

  bool DataObjectBase::hasSamePath(const std::shared_ptr<DataObjectBase>& dataObject)
  {
    // Can not be the same
    if (dimPaths_.size() != dataObject->dimPaths_.size())
    {
      return false;
    }

    bool isSame = true;
    for (size_t pathIdx = 0; pathIdx < dimPaths_.size(); ++pathIdx)
    {
      if (dimPaths_[pathIdx] !=  dataObject->dimPaths_[pathIdx])
      {
        isSame = false;
        break;
      }
    }

    return isSame;
  }

  void DataObjectBase::setFieldName(const std::string& fieldName)
  {
    fieldName_ = fieldName;
  }

  void DataObjectBase::setGroupByFieldName(const std::string& fieldName)
  {
    groupByFieldName_ = fieldName;
  }

  void DataObjectBase::setDims(const std::vector<int> dims)
  {
    dims_ = dims;
  }

  void DataObjectBase::setQuery(const std::string& query)
  {
    query_ = query;
  }

  void DataObjectBase::setDimPaths(const std::vector<Query>& dimPaths)
  {
    dimPaths_ = dimPaths;
  }

  void DataObjectBase::syncDimPaths(const eckit::mpi::Comm& comm, size_t numDims)
  {
    std::vector<std::string> dimPathsStr(dimPaths_.size());
    for (size_t i = 0; i < dimPaths_.size(); ++i)
    {
      dimPathsStr[i] = dimPaths_[i].str();
    }

    const int numLocalPaths = static_cast<int>(dimPathsStr.size());
    std::vector<int> pathCounts(comm.size());
    comm.allGather(numLocalPaths, pathCounts.begin(), pathCounts.end());

    std::vector<int> sizeDisplacement(comm.size(), 0);
    for (size_t i = 1; i < comm.size(); ++i)
    {
      sizeDisplacement[i] = sizeDisplacement[i - 1] + pathCounts[i - 1];
    }

    std::vector<int> localPathStringSizes(numLocalPaths);
    int charsToSend = 0;
    for (size_t i = 0; i < dimPathsStr.size(); ++i)
    {
      localPathStringSizes[i] = static_cast<int>(dimPathsStr[i].size());
      charsToSend += localPathStringSizes[i];
    }

    const int totalNumPaths = std::accumulate(pathCounts.begin(), pathCounts.end(), 0);
    std::vector<int> gatheredPathStringSizes(totalNumPaths);
    comm.gatherv(localPathStringSizes, gatheredPathStringSizes, pathCounts, sizeDisplacement, 0);

    std::vector<int> charCounts(comm.size());
    comm.allGather(charsToSend, charCounts.begin(), charCounts.end());

    std::vector<int> charDisplacement(comm.size(), 0);
    for (size_t i = 1; i < comm.size(); ++i)
    {
      charDisplacement[i] = charDisplacement[i - 1] + charCounts[i - 1];
    }

    std::vector<char> localChars;
    localChars.reserve(charsToSend);
    for (const auto& path : dimPathsStr)
    {
      localChars.insert(localChars.end(), path.begin(), path.end());
    }

    const int totalChars = std::accumulate(charCounts.begin(), charCounts.end(), 0);
    std::vector<char> gatheredChars(totalChars);
    comm.gatherv(localChars, gatheredChars, charCounts, charDisplacement, 0);

    if (comm.rank() != 0)
    {
      return;
    }

    int pathSizeOffset = 0;
    int charOffset = 0;
    for (size_t rank = 0; rank < comm.size(); ++rank)
    {
      const int candidateSize = pathCounts[rank];
      if (candidateSize != static_cast<int>(numDims))
      {
        for (int idx = 0; idx < candidateSize; ++idx)
        {
          charOffset += gatheredPathStringSizes[pathSizeOffset + idx];
        }
        pathSizeOffset += candidateSize;
        continue;
      }

      std::vector<Query> syncedDimPaths;
      syncedDimPaths.reserve(candidateSize);

      for (int idx = 0; idx < candidateSize; ++idx)
      {
        const int querySize = gatheredPathStringSizes[pathSizeOffset + idx];
        std::string queryStr(gatheredChars.begin() + charOffset,
                             gatheredChars.begin() + charOffset + querySize);

        auto parsed = QueryParser::parse(queryStr);
        if (parsed.empty())
        {
          std::ostringstream errMsg;
          errMsg << "Unable to parse dimPath query string: " << queryStr;
          throw eckit::BadParameter(errMsg.str());
        }

        syncedDimPaths.push_back(parsed.front());
        charOffset += querySize;
      }

      dimPaths_ = std::move(syncedDimPaths);
      return;
    }
  }
}  // namespace bufr
