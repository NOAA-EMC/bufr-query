/*
* (C) Copyright 2022 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include "bufr/ResultSet.h"

#include "ResultSetImpl.h"

namespace bufr {

  ResultSet::ResultSet() : impl_(std::make_unique<ResultSetImpl>()) {}

  ResultSet::ResultSet(const ResultSet& other) :
    impl_(std::make_unique<ResultSetImpl>(*other.impl_)) {}

  ResultSet::~ResultSet() = default;

  std::shared_ptr<DataObjectBase> ResultSet::get(const std::string& fieldName,
                                                 const std::string& groupByFieldName,
                                                 const std::string& overrideType) const
  {
        return impl_->get(fieldName, groupByFieldName, overrideType);
  }

}  // namespace bufr
