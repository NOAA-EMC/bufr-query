/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>

#include "ioda/ObsGroup.h"
#include "bufr/IodaEncoder.h"
#include "bufr/IodaDescription.h"
#include "bufr/DataContainer.h"

namespace py = pybind11;


using ioda::ObsGroup;
using bufr::IodaEncoder;
using bufr::IodaDescription;
using bufr::DataContainer;

void setupIodaEncoder(py::module& m)
{
  py::class_<IodaEncoder>(m, "IodaEncoder")
   .def(py::init<const std::string&>())
   .def(py::init<const IodaDescription&>())
   .def("encode", [](IodaEncoder& self,
                     const std::shared_ptr<DataContainer>& container) ->
      std::map<py::tuple, ioda::ObsGroup>
      {
        auto encodedData = self.encode(container);

        // std::vector<std::string> are not hashable in python (can't make a dict), so lets convert
        // it to a tuple instead
        std::map<py::tuple, ioda::ObsGroup> pyEncodedData;
        for (auto& [key, value] : encodedData)
        {
          pyEncodedData[py::cast(key)] = value;
        }

        return pyEncodedData;
      },
      py::arg("container"),
      "Get the class to encode the dataset");
}

