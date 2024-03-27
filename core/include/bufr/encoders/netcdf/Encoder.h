/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>

#include "eckit/config/LocalConfiguration.h"
//#include "ioda/Group.h"
//#include "ioda/Engines/EngineUtils.h"
//#include "ioda/ObsGroup.h"

#include <netcdf>

#include "bufr/DataContainer.h"
#include "Description.h"

namespace nc = netCDF;


namespace bufr {
namespace encoders {
namespace netcdf {
    /// \brief Uses IodaDescription and parsed data to create IODA data.
    class Encoder
    {
    public:
        struct Backend
        {
            bool isMemoryFile;
            std::string path;

            Backend() : isMemoryFile(false), path("") {};

            Backend(bool isInMemory, const std::string &backendPath) :
                isMemoryFile(isInMemory),
                path(backendPath) {};
        };

        explicit Encoder(const std::string &yamlPath);

        explicit Encoder(const Description &description);

        explicit Encoder(const eckit::Configuration &conf);

        /// \brief Encode the data into an ioda::ObsGroup object
        /// \param data The data container to use
        /// \param append Add data to existing file?
        std::map<SubCategory, std::shared_ptr<nc::NcFile>>
            encode(const std::shared_ptr<DataContainer> &data,
                   const Backend &backend = Backend(),
                   bool append = false);

    private:
        typedef std::map<std::vector<Query>, DimensionDescription> NamedPathDims;

        /// \brief The description
        const Description description_;

        /// \brief Create a string from a template string.
        /// \param prototype A template string ex: "my {dogType} barks". Sections labeled {__key__}
        ///        are treated as keys into the dictionary that defines their replacment values.
        std::string makeStrWithSubstitions(const std::string &prototype,
                                           const std::map<std::string, std::string> &subMap);

        /// \brief Used to find indicies of { and } by the makeStrWithSubstitions method.
        /// \param str Template string to search.
        std::vector<std::pair<std::string, std::pair<int, int>>>
        findSubIdxs(const std::string &str);

        /// \brief Check if the subquery string is a named dimension.
        /// \param path The subquery string to check.
        /// \param pathMap The map of named dimensions.
        /// \return True if the subquery string is a named dimension.
        bool existsInNamedPath(const Query &path, const NamedPathDims &pathMap) const;

        /// \brief Get the description associated with the named dimension.
        /// \param path The subquery string for the dimension.
        /// \param pathMap The map of named dimensions.
        /// \return The dimension description associated with the named dimension.
        DimensionDescription dimForDimPath(const Query &path,
                                           const NamedPathDims &pathMap) const;

        std::pair<std::string, std::string> splitName(std::string name) const;
    };
}  // namespace netcdf
}  // namespace encoders
}  // namespace bufr