/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>

namespace py = pybind11;

void setupParser(py::module& m);
void setupFile(py::module& m);
void setupQuerySet(py::module& m);
void setupResultSet(py::module& m);
void setupNetcdfDescription(py::module& m);
void setupNetcdfEncoder(py::module& m);
void setupDataContainer(py::module& m);
void setupDataCache(py::module& m);

PYBIND11_MODULE(bufr_python, m)
{
  m.doc() = "Python bindings for the bufr library";

  setupDataContainer(m);
  setupQuerySet(m);
  setupFile(m);
  setupResultSet(m);
  setupParser(m);
  setupDataCache(m);

  auto encoder_m = m.def_submodule("encoders", "BUFR data Encoders");

  auto netcdf_encoder_m = encoder_m.def_submodule("netcdf", "NetCDF4 Encoder");
  setupNetcdfDescription(netcdf_encoder_m);
  setupNetcdfEncoder(netcdf_encoder_m);
}
