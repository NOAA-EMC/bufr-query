// (C) Copyright 2024 NOAA/NWS/NCEP/EMC

#pragma once

#include "eckit/log/Log.h"

namespace bufr {
namespace log {

inline std::ostream &info() { return eckit::Log::info(); }
inline std::ostream &error() { return eckit::Log::error(); }
inline std::ostream &warning() { return eckit::Log::warning(); }
inline std::ostream &debug() { return eckit::Log::debug(); }

}  // namespace log
}  // namespace bufr
