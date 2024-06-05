/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>

#include <string>

#include "bufr/encoders/Description.h"


namespace py = pybind11;

using bufr::encoders::Description;

void setupEncoderDescription(py::module& m)
{
  py::class_<Description>(m, "Description")
   .def(py::init<const std::string&>())
   .def("add_variable", &Description::py_addVariable,
        py::arg("name"),
        py::arg("source"),
        py::arg("units"),
        py::arg("longName") = "", "");
}
