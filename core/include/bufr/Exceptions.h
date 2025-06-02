// (C) Copyright 2025 NOAA/NWS/NCEP/EMC

#pragma once

#include <stdexcept>
#include <string>

namespace bufr {

class Exception : public std::runtime_error {
 public:
  explicit Exception(const std::string& what) : std::runtime_error(what) {}
};

class BadParameter : public Exception {
 public:
  explicit BadParameter(const std::string& what) : Exception(what) {}
};

class BadValue : public Exception {
 public:
  explicit BadValue(const std::string& what) : Exception(what) {}
};

class MissingData : public Exception {
 public:
  explicit MissingData(const std::string& what) : Exception(what) {}
};

}  // namespace bufr

