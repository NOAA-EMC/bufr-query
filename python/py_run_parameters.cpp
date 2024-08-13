/*
* (C) Copyright 2024 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <chrono>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

#include "bufr/DataProvider.h"

namespace py = pybind11;

using bufr::RunParameters;

void setupRunParameters(py::module& m)
{
  py::class_<RunParameters>(m, "RunParameters")
    .def(py::init<>())
    .def_readwrite("offset", &RunParameters::offset)
    .def_readwrite("num_messages", &RunParameters::numMessages)
    .def_property("start_time",
                  [](const RunParameters &rp) -> py::object {
                    if (rp.startTime)
                    {
                      return py::cast(std::chrono::system_clock::from_time_t(*rp.startTime));
                    }
                    else
                    {
                      return py::none();
                    }
                  },
                  [](RunParameters &rp, const std::chrono::system_clock::time_point &tp) {
                    rp.startTime = std::chrono::system_clock::to_time_t(tp);
                  })
    .def_property("stop_time",
                  [](const RunParameters &rp) -> py::object {
                    if (rp.stopTime)
                    {
                      return py::cast(std::chrono::system_clock::from_time_t(*rp.stopTime));
                    }
                    else
                    {
                      return py::none();
                    }
                  },
                  [](RunParameters &rp, const std::chrono::system_clock::time_point &tp) {
                    rp.stopTime = std::chrono::system_clock::to_time_t(tp);
                  });
}
