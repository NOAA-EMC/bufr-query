// (C) Copyright 2025 NOAA/NWS/NCEP/EMC

#pragma once

#include "Transform.h"


namespace bufr {
  /// \brief Transforms data by multiplying it by a scaling factor.
  class WrapTransform : public Transform
  {
  public:
    /// \brief Constructor
    WrapTransform(const std::vector<float> range);
    ~WrapTransform() = default;

    /// \brief Modify data according to the rules of the transform.
    /// \param array Array of data to modify.
    void apply(std::shared_ptr<DataObjectBase>& dataObject) override;

   private:
     std::vector<float> range_;
  };
}  // namespace bufr
