/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <netcdf>

#include "bufr/DataContainer.h"
#include "bufr/encoders/netcdf/Description.h"
#include "bufr/encoders/netcdf/Encoder.h"


namespace py = pybind11;
namespace nc = netCDF;

using bufr::DataContainer;
using bufr::encoders::netcdf::Encoder;
using bufr::encoders::netcdf::Description;

void setupNetcdfEncoder(py::module& m)
{
  py::class_<Encoder>(m, "Encoder")
   .def(py::init<const std::string&>())
   .def(py::init<const Description&>())
   .def("encode", [](Encoder& self,
                     const std::shared_ptr<DataContainer>& container) ->
      std::map<py::tuple, std::shared_ptr<nc::NcFile>>
      {
        auto encodedData = self.encode(container);

        // std::vector<std::string> are not hashable in python (can't make a dict), so lets convert
        // it to a tuple instead
        std::map<py::tuple, std::shared_ptr<nc::NcFile>> pyEncodedData;
        for (auto& [key, value] : encodedData)
        {
          pyEncodedData[py::cast(key)] = value;
        }

        return pyEncodedData;
      },
      py::arg("container"),
      "Get the class to encode the dataset");
}
