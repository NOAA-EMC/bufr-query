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

#include "bufr/QuerySet.h"

namespace py = pybind11;

using bufr::QuerySet;

void setupQuerySet(py::module& m)
{
  py::class_<QuerySet>(m, "QuerySet")
   .def(py::init<>())
   .def(py::init<const std::vector<std::string>&>())
   .def("size", &QuerySet::size, "Get the number of queries in the query set.")
   .def("add", &QuerySet::add, "Add a query to the query set.");

}
