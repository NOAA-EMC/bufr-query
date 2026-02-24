// (C) Copyright 2024 NOAA/NWS/NCEP/EMC

#include "bufr/DataObject.h"
#include "bufr/Data.h"

namespace bufr {

  namespace {
    std::vector<int> computeDisplacements(const std::vector<int>& counts)
    {
      std::vector<int> displacements(counts.size(), 0);
      for (size_t i = 1; i < counts.size(); ++i)
      {
        displacements[i] = displacements[i - 1] + counts[i - 1];
      }

      return displacements;
    }

    std::vector<std::string> serializeDimPaths(const std::vector<Query>& dimPaths)
    {
      std::vector<std::string> serialized(dimPaths.size());
      for (size_t i = 0; i < dimPaths.size(); ++i)
      {
        serialized[i] = dimPaths[i].str();
      }

      return serialized;
    }

    std::vector<char> packStrings(const std::vector<std::string>& strings,
                                  std::vector<int>& stringSizes,
                                  int& totalChars)
    {
      stringSizes.resize(strings.size());
      totalChars = 0;
      for (size_t i = 0; i < strings.size(); ++i)
      {
        stringSizes[i] = static_cast<int>(strings[i].size());
        totalChars += stringSizes[i];
      }

      std::vector<char> packed;
      packed.reserve(totalChars);
      for (const auto& str : strings)
      {
        packed.insert(packed.end(), str.begin(), str.end());
      }

      return packed;
    }

    bool selectSyncedDimPaths(const std::vector<int>& pathCounts,
                              const std::vector<int>& gatheredPathStringSizes,
                              const std::vector<char>& gatheredChars,
                              size_t numDims,
                              std::vector<Query>& syncedDimPaths)
    {
      int pathSizeOffset = 0;
      int charOffset = 0;
      for (size_t rank = 0; rank < pathCounts.size(); ++rank)
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

        syncedDimPaths.clear();
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

        return true;
      }

      return false;
    }

    void syncDimPathsImpl(const eckit::mpi::Comm& comm,
                          size_t numDims,
                          std::vector<Query>& dimPaths,
                          bool useAllGather)
    {
      const auto dimPathsStr = serializeDimPaths(dimPaths);

      const int numLocalPaths = static_cast<int>(dimPathsStr.size());
      std::vector<int> pathCounts(comm.size());
      comm.allGather(numLocalPaths, pathCounts.begin(), pathCounts.end());

      const auto sizeDisplacement = computeDisplacements(pathCounts);
      const int totalNumPaths = std::accumulate(pathCounts.begin(), pathCounts.end(), 0);

      std::vector<int> localPathStringSizes;
      int charsToSend = 0;
      const auto localChars = packStrings(dimPathsStr, localPathStringSizes, charsToSend);

      std::vector<int> gatheredPathStringSizes(totalNumPaths);
      if (useAllGather)
      {
        comm.allGatherv(localPathStringSizes.begin(), localPathStringSizes.end(),
                        gatheredPathStringSizes.begin(), pathCounts.data(),
                        sizeDisplacement.data());
      }
      else
      {
        comm.gatherv(localPathStringSizes, gatheredPathStringSizes,
                     pathCounts, sizeDisplacement, 0);
      }

      std::vector<int> charCounts(comm.size());
      comm.allGather(charsToSend, charCounts.begin(), charCounts.end());
      const auto charDisplacement = computeDisplacements(charCounts);
      const int totalChars = std::accumulate(charCounts.begin(), charCounts.end(), 0);

      std::vector<char> gatheredChars(totalChars);
      if (useAllGather)
      {
        comm.allGatherv(localChars.begin(), localChars.end(), gatheredChars.begin(),
                        charCounts.data(), charDisplacement.data());
      }
      else
      {
        comm.gatherv(localChars, gatheredChars, charCounts, charDisplacement, 0);
        if (comm.rank() != 0)
        {
          return;
        }
      }

      std::vector<Query> syncedDimPaths;
      if (selectSyncedDimPaths(pathCounts,
                               gatheredPathStringSizes,
                               gatheredChars,
                               numDims,
                               syncedDimPaths))
      {
        dimPaths = std::move(syncedDimPaths);
      }
    }
  }  // namespace

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
    syncDimPathsImpl(comm, numDims, dimPaths_, false);
  }

  void DataObjectBase::syncDimPathsAllGather(const eckit::mpi::Comm& comm, size_t numDims)
  {
    syncDimPathsImpl(comm, numDims, dimPaths_, true);
  }
}  // namespace bufr
