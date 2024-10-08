/*
* (C) Copyright 2023 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <memory>
#include <vector>
#include <string>

#include "bufr/DataObject.h"
#include "bufr/DataContainer.h"
#include "bufr/Tokenizer.h"
#include "bufr/QueryParser.h"

#include "DataObjectFunctions.h"
#include "py_mpi.h"


namespace py = pybind11;

using bufr::DataContainer;
using bufr::DataObjectBase;
using bufr::CategoryMap;
using bufr::SubCategory;
using bufr::Query;
using bufr::QueryParser;


void setupDataContainer(py::module& m)
{
  py::class_<DataContainer, std::shared_ptr<DataContainer>>(m, "DataContainer")
   .def(py::init<>())
   .def(py::init<const CategoryMap&>())
   .def("add", [](DataContainer& self,
                  const std::string& fieldName,
                  const py::array& pyData,
                  const std::vector<std::string>& dimPaths,
                  const SubCategory& categoryId = {})
        {
          // Guard statements
          if (!self.hasCategory(categoryId))
          {
            std::ostringstream errorStr;
            errorStr << "ERROR: Invalid category " << self.makeSubCategoryStr(categoryId);
            errorStr << " for field " << fieldName << "." << std::endl;
            throw eckit::BadParameter(errorStr.str());
          }

          if (self.hasKey(fieldName, categoryId))
          {
            std::ostringstream errorStr;
            errorStr << "ERROR: Field called " << fieldName << " already exists ";
            errorStr << "for subcategory " << self.makeSubCategoryStr(categoryId) << std::endl;
            throw eckit::BadParameter(errorStr.str());
          }

          auto paths = std::vector<Query>(dimPaths.size());
          for (size_t pathIdx = 0; pathIdx < dimPaths.size(); pathIdx++)
          {
            paths[pathIdx] = QueryParser::parse(dimPaths[pathIdx])[0];
          }

          auto dataObj = bufr::makeObject(fieldName, pyData);
          dataObj->setDimPaths(paths);
          self.add(fieldName, dataObj, categoryId);
        },

        py::arg("name"),
        py::arg("data"),
        py::arg("dim_paths"),
        py::arg("category") = std::vector<std::string>(),
        "Add a new variable object into the data container.")
   .def("get", [](DataContainer& self,
                  const std::string& fieldName,
                  const SubCategory& categoryId = {})
        {
          return bufr::pyArrayFromObj(self.get(fieldName, categoryId));
        },
        py::arg("name"),
        py::arg("category") = std::vector<std::string>(),
        "Get the value of the variable object as numpy array. ")
   .def("get_paths", &DataContainer::getPaths,
        py::arg("name"),
        py::arg("category") = std::vector<std::string>(),
        "Get path names for a field.")
   .def("replace", [](DataContainer& self,
                      const std::string& fieldName,
                      const py::array& pyData,
                      const SubCategory& categoryId)
        {
          // Guard statements
          if (!self.hasKey(fieldName, categoryId))
          {
            throw eckit::BadParameter("ERROR: Field " + fieldName +  " does not exist.");
          }

          const auto& data = self.get(fieldName, categoryId);

          if (pyData.ndim() != data->getDims().size())
          {
            throw eckit::BadParameter("ERROR: Dimension mismatch.");
          }

          for (size_t idx = 0; idx < pyData.ndim(); idx++)
          {
            if (pyData.shape(idx) != data->getDims()[idx])
            {
              throw eckit::BadParameter("ERROR: Dimension mismatch.");
            }
          }

          auto dataObj = bufr::makeObject(fieldName, pyData);
          dataObj->setDimPaths(data->getDimPaths());
          self.set(dataObj, fieldName, categoryId);
        },
        py::arg("name"),
        py::arg("data"),
        py::arg("category") = std::vector<std::string>(),
        "Replace the variable with the given name.")
   .def("get_category_map", &DataContainer::getCategoryMap, "Get the map.")
   .def("all_sub_categories", &DataContainer::allSubCategories,
        "Get the sub categories.")
   .def("get_sub_container", &DataContainer::getSubContainer,
        py::arg("category"),
        "Get the data container for the sub category.")
   .def("list", &DataContainer::getFieldNames, "Get the field names.")
   .def("append", &DataContainer::append,
        py::arg("other"),
        "Append contents of another container. Must have the same category map and fields.")
    .def("deduplicate", &DataContainer::deduplicate,
         py::arg("dedupFields"),
         "Remove duplicate rows.")
   .def("gather", [](DataContainer& self, bufr::mpi::Comm& comm)
        {
          return self.gather(comm.getComm());
        },
        py::arg("comm"),
        "Gather data from all tasks into rank 0 task.")
   .def("all_gather", [](DataContainer& self, bufr::mpi::Comm& comm)
        {
          return self.allGather(comm.getComm());
        },
        py::arg("comm"),
        "Gather data from all tasks into all tasks. Each task will have the complete record.");
}
