// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include "Transform.h"


namespace bufr {
    /// \brief Transforms data by multiplying it by a scaling factor.
    class ScalingTransform : public Transform
    {
     public:
        /// \brief Constructor
        /// \param scaling Value to multiply by.
        explicit ScalingTransform(const double scaling);
        ~ScalingTransform() = default;

        /// \brief Modify data according to the rules of the transform.
        /// \param array Array of data to modify.
        void apply(std::shared_ptr<DataObjectBase>& dataObject) override;

     private:
        const double scaling_;
    };
}  // namespace bufr
