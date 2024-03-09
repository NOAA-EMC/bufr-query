/*
* (C) Copyright 2024 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#pragma once

#include "eckit/log/Log.h"

namespace bufr {
namespace log {

static std::ostream &info() { return eckit::Log::info(); }
static std::ostream &error() { return eckit::Log::error(); }
static std::ostream &warning() { return eckit::Log::warning(); }
static std::ostream &debug() { return eckit::Log::debug(); }

}  // namespace log
}  // namespace bufr
