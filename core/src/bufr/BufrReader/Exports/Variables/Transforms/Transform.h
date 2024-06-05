// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include <memory>
#include <vector>

#include "bufr/DataObject.h"

namespace bufr {
    /// \brief Base class of all transform classes. Classes are used to transform data.
    class Transform
    {
     public:
        virtual ~Transform() = default;

        /// \brief Modify data according to the rules of the transform.
        /// \param array Array of data to modify.
        virtual void apply(std::shared_ptr<DataObjectBase>& dataObject) = 0;
    };

    typedef std::vector <std::shared_ptr<Transform>> Transforms;
}  // namespace bufr
