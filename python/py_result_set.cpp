/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "bufr/DataObject.h"
#include "bufr/ResultSet.h"

#include "DataObjectFunctions.h"


namespace py = pybind11;

using bufr::ResultSet;
using bufr::DataObjectBase;

void setupResultSet(py::module& m)
{
 py::class_<ResultSet>(m, "ResultSet")
   .def("get", [](const ResultSet& self,
                  const std::string& field_name,
                  const std::string& group_by,
                  const std::string& type)
        {
          return bufr::pyArrayFromObj(self.get(field_name, group_by, type));
        },

        py::arg("field_name"),
        py::arg("group_by") = std::string(""),
        py::arg("type") = std::string(""),
        "Get a numpy array of the specified field name. If the group_by "
        "field is specified, the array is grouped by the specified field."
        "It is also possible to specify a type to override the default type.")
   .def("get_datetime", [](const ResultSet& self,
                           const std::string& year,
                           const std::string& month,
                           const std::string& day,
                           const std::string& hour,
                           const std::string& minute,
                           const std::string& second,
                           const std::string& groupBy)
        {
          auto yearObj  = self.get(year, groupBy);
          auto monthObj = self.get(month, groupBy);
          auto dayObj   = self.get(day, groupBy);
          auto hourObj  = self.get(hour, groupBy);

           std::shared_ptr<DataObjectBase> minuteObj = nullptr;
           std::shared_ptr<DataObjectBase> secondObj = nullptr;

           if (!minute.empty()) {
             minuteObj = self.get(minute, groupBy);
           }

           if (!second.empty()) {
             secondObj = self.get(second, groupBy);
           }

           // make strides array
           std::vector<size_t> strides(yearObj->getDims().size());
           strides[0] = sizeof(int64_t);
           for (size_t i = 1; i < yearObj->getDims().size(); ++i) {
             strides[i] = sizeof(int64_t) * yearObj->getDims()[i];
           }

           auto array    = py::array(py::dtype("datetime64[s]"), yearObj->getDims(), strides);
           auto arrayPtr = static_cast<int64_t*>(array.mutable_data());

           for (size_t i = 0; i < yearObj->size(); ++i)
           {
             std::tm time;
             time.tm_year  = yearObj->getAsInt(i) - 1900;
             time.tm_mon   = monthObj->getAsInt(i) - 1;
             time.tm_mday  = dayObj->getAsInt(i);
             time.tm_hour  = hourObj->getAsInt(i);
             time.tm_min   = minuteObj ? minuteObj->getAsInt(i) : 0;
             time.tm_sec   = secondObj ? secondObj->getAsInt(i) : 0;
             time.tm_isdst = 0;

             arrayPtr[i] = static_cast<int64_t>(timegm(&time));
           }

           // Create the mask array
           py::object numpyModule = py::module::import("numpy");

           // Create the mask array
           py::array_t<bool> mask(yearObj->getDims());
           bool* maskPtr = static_cast<bool*>(mask.mutable_data());
           for (size_t idx = 0; idx < yearObj->size(); idx++)
           {
             maskPtr[idx] = yearObj->isMissing(idx) ||
                            monthObj->isMissing(idx) ||
                            dayObj->isMissing(idx) ||
                            hourObj->isMissing(idx) ||
                            (minuteObj ? minuteObj->isMissing(idx) : false) ||
                            (secondObj ? secondObj->isMissing(idx) : false);
           }

           // Create a masked array from the data and mask arrays
           py::array maskedArray = numpyModule.attr("ma").attr("masked_array")(array, mask);
           numpyModule.attr("ma").attr("set_fill_value")(maskedArray, 0);

           return maskedArray;
        },
        py::arg("year"),
        py::arg("month"),
        py::arg("day"),
        py::arg("hour"),
        py::arg("minute") = std::string(""),
        py::arg("second") = std::string(""),
        py::arg("group_by") = std::string(""),
        "Get a numpy array of datetime objects. The datetime objects are "
        "constructed from the specified year, month, day, hour, minute, "
        "and second fields. If the minute and second fields are not "
        "specified, they are assumed to be 0. If the group_by field is "
        "specified, the datetime objects are grouped by the specified "
        "field.");
}
