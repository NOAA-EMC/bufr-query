/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "DataObject.h"
#include "Eigen/Dense"

namespace bufr {
  typedef std::unordered_map<std::string, std::shared_ptr<DataObjectBase> > BufrDataMap;
}  // namespace bufr
