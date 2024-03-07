/*
* (C) Copyright 2024 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include "bufr/DataObject.h"
#include "bufr/Data.h"

namespace bufr {

  bool DataObjectBase::hasSamePath(const std::shared_ptr<DataObjectBase>& dataObject)
  {
    // Can't be the same
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
}  // namespace bufr
