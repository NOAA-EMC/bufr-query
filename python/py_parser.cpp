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
#include <mpi.h>

#include "bufr/BufrParser.h"
#include "bufr/DataProvider.h"

#include "py_mpi.h"

namespace py = pybind11;

using bufr::BufrParser;
using bufr::RunParameters;

void setupParser(py::module& m)
{
  m.doc() = "Provides the ability to process data from BUFR files.";

  py::class_<RunParameters>(m, "RunParameters")
    .def(py::init<>())
    .def_readwrite("offset", &RunParameters::offset)
    .def_readwrite("num_messages", &RunParameters::numMessages)
    .def_readwrite("start_time", &RunParameters::startTime)
    .def_readwrite("stop_time", &RunParameters::stopTime);

  py::class_<BufrParser>(m, "Parser")
    .def(py::init<const std::string&, const std::string&, const std::string&>(),
         py::arg("obsfile"),
         py::arg("mapping_path"),
         py::arg("table_path") = "")
    .def("parse", [](BufrParser& self, const RunParameters& params)
         {
           return self.parse(params);
         },
         py::arg("params") = RunParameters(),
         "Get Parser to parse a config file and get the data container.")
    .def("parse", [](BufrParser& self, bufr::mpi::Comm& comm, const RunParameters& params)
        {
          return self.parse(comm.getComm(), params);
        },
        py::arg("comm"),
        py::arg("params") = RunParameters(),
        "Get Parser to parse a config file and get the data container in parallel.");
}
