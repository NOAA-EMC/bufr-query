// (C) Copyright 2025 NOAA/NWS/NCEP/EMC

#include "WrapTransform.h"

namespace bufr {
  WrapTransform::WrapTransform(const std::vector<float> range) :
    range_(range)
  {
  }

  void WrapTransform::apply(std::shared_ptr<DataObjectBase>& dataObject)
  {
    dataObject->wrap(range_);
  }
}  // namespace bufr
