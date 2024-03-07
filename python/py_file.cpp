/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>

#include <memory>
#include <string>

#include "bufr/File.h"

namespace py = pybind11;

using bufr::File;

void setupFile(py::module& m)
{
  py::class_<File>(m, "File")
   .def(py::init<const std::string&, const std::string&>(),
        py::arg("filename"),
        py::arg("wmoTablePath") = std::string(""))
   .def("execute", &File::execute,
        py::arg("query_set"),
        py::arg("next") = static_cast<int>(0),
        "Execute a query set on the file. Returns a ResultSet object.")
   .def("rewind", &File::rewind, "Rewind the file to the beginning.")
   .def("close", &File::close, "Close the file.")
   .def("__enter__", [](File &f) { return &f; })
   .def("__exit__", [](File &f, py::args args) { f.close(); });
}
