/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>

#include <memory>
#include <vector>
#include <string>

#include "bufr/IodaDescription.h"

namespace py = pybind11;

using bufr::IodaDescription;

void setupIodaDescription(py::module& m)
{
  py::class_<IodaDescription>(m, "IodaDescription")
   .def(py::init<const std::string&>())
   .def("add_variable", &IodaDescription::py_addVariable,
        py::arg("name"),
        py::arg("source"),
        py::arg("units"),
        py::arg("longName") = "", "");
}
