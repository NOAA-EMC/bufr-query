// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#include "ScalingTransform.h"

namespace bufr {
    ScalingTransform::ScalingTransform(const double scaling) :
      scaling_(scaling)
    {
    }

    void ScalingTransform::apply(std::shared_ptr<DataObjectBase>& dataObject)
    {
        dataObject->multiplyBy(scaling_);
    }
}  // namespace bufr
