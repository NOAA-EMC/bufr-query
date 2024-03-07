/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <vector>
#include <string>

#include "bufr/DataCache.h"

namespace py = pybind11;

using bufr::DataCache;

void setupDataCache(py::module& m)
{
  py::class_<DataCache>(m, "DataCache")
      .def_static("has", &DataCache::has,
           py::arg("src_path"),
           py::arg("map_path"),
           "Does the cache contain data for the given paths.")
      .def_static("get", &DataCache::get,
           py::arg("src_path"),
          py::arg("map_path"),
          "Get the DataContainer for the given paths.")
      .def_static("add", &DataCache::add,
           py::arg("src_path"),
           py::arg("map_path"),
           py::arg("cached_categories"),
           py::arg("data"),
           "Add a DataContainer to the cache.")
      .def_static("mark_finished", &DataCache::markFinished,
           py::arg("src_path"),
           py::arg("map_path"),
           py::arg("category"),
           "Mark a category as finished. Delete entry when all \"cachedCategories\" are finished.");
}
