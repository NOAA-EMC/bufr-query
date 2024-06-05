// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#include "OffsetTransform.h"

#include "bufr/BufrTypes.h"

namespace bufr {
    OffsetTransform::OffsetTransform(const double offset) :
      offset_(offset)
    {
    }

    void OffsetTransform::apply(std::shared_ptr<DataObjectBase>& dataObject)
    {
        dataObject->offsetBy(offset_);
    }

}  // namespace bufr
