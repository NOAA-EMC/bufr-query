#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <regex>

#include "bufr/DataObject.h"

namespace py = pybind11;

namespace bufr {

static const std::regex strRegex("[|\\<\\>]?[US]\\d*");

// Return the missing value constant for the provided numpy dtype.
py::object getMissingValue(const py::object& dtypeObj) {
    py::dtype dt = py::dtype::from_args(dtypeObj);
    std::string dtypeStr = py::cast<std::string>(py::str(dt));
    std::cmatch m;

    if (dt.is(py::dtype::of<int64_t>())) {
        return py::int_(DataObject<int64_t>::missingValue());
    } else if (dt.is(py::dtype::of<int>())) {
        return py::int_(DataObject<int>::missingValue());
    } else if (dt.is(py::dtype::of<double>())) {
        return py::float_(DataObject<double>::missingValue());
    } else if (dt.is(py::dtype::of<float>())) {
        return py::float_(DataObject<float>::missingValue());
    } else if (dtypeStr == "object" || std::regex_match(dtypeStr.c_str(), m, strRegex)) {
        return py::str(DataObject<std::string>::missingValue());
    }

    throw std::runtime_error("Unsupported dtype for get_missing_value");
}

void setupHelpers(py::module& m) {
    m.def("get_missing_value", &getMissingValue,
          py::arg("dtype"),
          "Return the missing value constant for the given numpy dtype.");
}

} // namespace bufr
