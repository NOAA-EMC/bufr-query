/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/filesystem/PathName.h"

#include "bufr/BufrDescription.h"

namespace
{
  namespace ConfKeys
  {
    const char* Exports = "exports";
  }  // namespace ConfKeys
}  // namespace

namespace bufr {
  BufrDescription::BufrDescription(const std::string& yamlPath) :
    export_(Export())
  {
    auto conf = eckit::YAMLConfiguration(eckit::PathName(yamlPath));
    auto bufrConf = conf.getSubConfiguration("bufr");
    export_ = Export(bufrConf.getSubConfiguration(ConfKeys::Exports));
  }

  BufrDescription::BufrDescription(const eckit::Configuration &conf) :
      export_(Export(conf.getSubConfiguration(ConfKeys::Exports)))
  {
  }
}  // namespace bufr
