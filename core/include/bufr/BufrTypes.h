// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "DataObject.h"

namespace bufr {
  typedef std::unordered_map<std::string, std::shared_ptr<DataObjectBase> > BufrDataMap;
}  // namespace bufr
