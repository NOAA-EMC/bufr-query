// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include "Transform.h"


namespace bufr {
    /// \brief Transforms data by adding an offset to it.
    class OffsetTransform : public Transform
    {
     public:
        /// \brief Constructor
        /// \param offset The value to add.
        explicit OffsetTransform(const double offset);
        ~OffsetTransform() = default;

        /// \brief Modify data according to the rules of the transform.
        /// \param array Array of data to modify.
        void apply(std::shared_ptr<DataObjectBase>& dataObject) override;

     private:
        const double offset_;
    };
}  // namespace bufr
