/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include "bufr/encoders/netcdf/Encoder.h"

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include <netcdf>

//#include "../ElementWriter.h"
#include "../../bufr/Log.h"
#include "bufr/DataObject.h"
#include "eckit/exception/Exceptions.h"
//#include "ioda/Layout.h"
//#include "ioda/Misc/DimensionScales.h"

namespace nc = netCDF;
namespace log = bufr::log;


namespace bufr {
namespace encoders {

//    template<typename T>
//    class VarWriter : public ElementWriter
//    {
//     public:
//        VarWriter(nc::NcGroup& group) : _group(group) {}
//
//        void write(const std::vector<T>& data)
//        {
//            group.putVar(data);
//        }
//
//     private:
//        nc::NcGroup& _group;
//    };

namespace netcdf {
    static const char* LocationName = "Location";
    static const char* DefualtDimName = "dim";

    Encoder::Encoder(const std::string &yamlPath) :
        description_(Description(yamlPath))
    {
    }

    Encoder::Encoder(const Description &description) :
        description_(description)
    {
    }

    Encoder::Encoder(const eckit::Configuration &conf) :
        description_(Description(conf))
    {
    }

    std::map<SubCategory, std::shared_ptr<nc::NcFile>>
    Encoder::encode(const std::shared_ptr<DataContainer> &dataContainer,
                    const Encoder::Backend &backend,
                    bool append)
    {
        std::map<SubCategory, std::shared_ptr<nc::NcFile>> obsGroups;

        // Get the named dimensions
        NamedPathDims namedLocDims;
        NamedPathDims namedExtraDims;

        // Get a list of all the named dimensions
        {
            std::set<std::string> dimNames;
            std::set<Query> dimPaths;
            for (const auto &dim: description_.getDims())
            {
                if (dimNames.find(dim.name) != dimNames.end())
                {
                    throw eckit::UserError("ioda::dimensions: Duplicate dimension name: "
                                           + dim.name);
                }

                dimNames.insert(dim.name);

                // Validate the dimension paths so that we don't have duplicates and they all start
                // with a *.
                for (auto path: dim.paths)
                {
                    if (dimPaths.find(path) != dimPaths.end())
                    {
                        throw eckit::BadParameter("ioda::dimensions: Declared duplicate dim. path: "
                                                  + path.str());
                    }

                    if (path.str().substr(0, 1) != "*")
                    {
                        std::ostringstream errStr;
                        errStr << "ioda::dimensions: ";
                        errStr << "Path " << path.str() << " must start with *. ";
                        errStr << "Subset specific named dimensions are not supported.";

                        throw eckit::BadParameter(errStr.str());
                    }

                    dimPaths.insert(path);
                }

                namedExtraDims.insert({dim.paths, dim});
            }
        }

//        auto backendParams = ioda::Engines::BackendCreationParameters();
//
        // Got through each unique category
        for (const auto &categories: dataContainer->allSubCategories())
        {
            // Create the dimensions variables
            std::map<std::string, std::shared_ptr<DimensionDataBase>> dimMap;

            auto dataObjectGroupBy = dataContainer->getGroupByObject(
                description_.getVariables()[0].source, categories);

            // When we find that the primary index is zero we need to skip this category
            if (dataObjectGroupBy->getDims()[0] == 0)
            {
                log::warning() << "  Category (";
                for (auto category: categories)
                {
                    log::warning() << category;

                    if (category != categories.back())
                    {
                        log::warning() << ", ";
                    }
                }

                log::warning() << ") was not found in file." << std::endl;
            }

            // Create the root Location dimension for this category
            auto rootDim = std::make_shared<DimensionData<int>>(LocationName,
                                                                dataObjectGroupBy->getDims()[0]);
            dimMap[LocationName] = rootDim;

            // Add the root Location dimension as a named dimension
            auto rootLocation = DimensionDescription();
            rootLocation.name = LocationName;
            rootLocation.source = "";
            namedLocDims[{dataObjectGroupBy->getDimPaths()[0]}] = rootLocation;

            // Create the dimension data for dimensions which include source data
            for (const auto &dimDesc: description_.getDims())
            {
                if (!dimDesc.source.empty())
                {
                    auto dataObject = dataContainer->get(dimDesc.source, categories);

                    // Validate the path for the source field makes sense for the dimension
                    if (std::find(dimDesc.paths.begin(),
                                  dimDesc.paths.end(),
                                  dataObject->getDimPaths().back()) == dimDesc.paths.end())
                    {
                        std::stringstream errStr;
                        errStr << "ioda::dimensions: Source field " << dimDesc.source << " in ";
                        errStr << dimDesc.name << " is not in the correct path.";
                        throw eckit::BadParameter(errStr.str());
                    }

                    // Create the dimension data
                    dimMap[dimDesc.name] = dataObject->createDimensionFromData(
                        dimDesc.name,
                        dataObject->getDimPaths().size() - 1);
                }
            }

            // Discover and create the dimension data for dimensions with no source field. If
            // dim is un-named (not listed) then call it dim_<number>
            int autoGenDimNumber = 2;
            for (const auto &varDesc: description_.getVariables())
            {
                auto dataObject = dataContainer->get(varDesc.source, categories);

                for (std::size_t dimIdx = 1; dimIdx < dataObject->getDimPaths().size(); dimIdx++)
                {
                    auto dimPath = dataObject->getDimPaths()[dimIdx];
                    std::string dimName = "";

                    if (existsInNamedPath(dimPath, namedExtraDims))
                    {
                        dimName = dimForDimPath(dimPath, namedExtraDims).name;
                    }
                    else
                    {
                        auto newDimStr = std::ostringstream();
                        newDimStr << DefualtDimName << "_" << autoGenDimNumber;

                        dimName = newDimStr.str();

                        auto dimDesc = DimensionDescription();
                        dimDesc.name = dimName;
                        dimDesc.source = "";

                        namedExtraDims[{dimPath}] = dimDesc;
                        autoGenDimNumber++;
                    }

                    if (dimMap.find(dimName) == dimMap.end())
                    {
                        dimMap[dimName] = dataObject->createEmptyDimension(dimName, dimIdx);
                    }
                }
            }

            // Make the categories
            size_t catIdx = 0;
            std::map<std::string, std::string> substitutions;
            for (const auto &catPair: dataContainer->getCategoryMap())
            {
                substitutions.insert({catPair.first, categories.at(catIdx)});
                catIdx++;
            }

            auto fileName = makeStrWithSubstitions(backend.path, substitutions);

            int fileFlags = NC_CLOBBER | NC_WRITE | NC_NETCDF4;
            if (backend.isMemoryFile)
            {
                fileFlags = fileFlags | NC_DISKLESS;
            }

            auto file = std::make_shared<nc::NcFile>();
            file->create(fileName, fileFlags);

            // Create the Globals
            for (auto &global: description_.getGlobals())
            {
                global->addTo(*file);
            }

            // Add Dimensions
            for (auto dimPair: dimMap)
            {
                const auto& dim = file->addDim(dimPair.first, dimPair.second->size());
                file->addVar(dimPair.first, nc::NcType::nc_INT, dim);
            }

            for (const auto& dimDesc : description_.getDims())
            {
                if (!dimDesc.source.empty())
                {
                    auto dataObject = dataContainer->get(dimDesc.source, categories);
                    for (size_t dimIdx = 0; dimIdx < dataObject->getDims().size(); dimIdx++)
                    {
                        auto dimPath = dataObject->getDimPaths()[dimIdx];

                        NamedPathDims namedPathDims;
                        if (dimIdx == 0)
                        {
                            namedPathDims = namedLocDims;
                        }
                        else
                        {
                            namedPathDims = namedExtraDims;
                        }

                        auto dimName = dimForDimPath(dimPath, namedPathDims).name;
                        auto dimVar = file->getVar(dimName);

                        if (const auto obj = std::dynamic_pointer_cast<DataObject<size_t>>(dataObject))
                        {
                            dimVar.putVar(obj->getRawData().data());
                        }
                        else
                        {
                            throw eckit::BadParameter("Dimension data type not supported.");
                        }

                        dimMap[dimName]->write(dimVar);
                    }
                }
            }

            // Write all the other Variables
            std::set<std::string> groupNames;
            for (const auto &varDesc: description_.getVariables())
            {
                auto[groupName, varName] = splitName(varDesc.name);
                if (groupNames.find(groupName) == groupNames.end())
                {
                    file->addGroup(groupName);
                    groupNames.insert(groupName);
                }

                auto group = file->getGroup(groupName);
                std::vector<size_t> chunks;
                auto dimNames = std::vector<std::string>();
                auto dataObject = dataContainer->get(varDesc.source, categories);
                for (size_t dimIdx = 0; dimIdx < dataObject->getDims().size(); dimIdx++)
                {
                    auto dimPath = dataObject->getDimPaths()[dimIdx];

                    NamedPathDims namedPathDims;
                    if (dimIdx == 0)
                    {
                        namedPathDims = namedLocDims;
                    } else
                    {
                        namedPathDims = namedExtraDims;
                    }

                    dimNames.push_back(dimForDimPath(dimPath, namedPathDims).name);

//                    auto dimVar = group.getVar(dimForDimPath(dimPath, namedPathDims).name);
//                    if (dimIdx < varDesc.chunks.size())
//                    {
//                        chunks.push_back(std::min(dimVar.getChunkSizes()[0],
//                                                  varDesc.chunks[dimIdx]));
//                    } else
//                    {
//                        chunks.push_back(dimVar.getChunkSizes()[0]);
//                    }
                }

                // Check that dateTime variable has the right dimensions
                if (varDesc.name == "MetaData/dateTime" || varDesc.name == "MetaData/datetime")
                {
                    if (dimNames.size() != 1)
                    {
                        throw eckit::BadParameter(
                            "IODA requires Datetime variable to be one dimensional.");
                    }
                }

                auto var = dataObject->createVariable(group,
                                                      varName,
                                                      dimNames,
                                                      chunks,
                                                      varDesc.compressionLevel);

                var.putAtt("long_name", varDesc.longName);
                if (!varDesc.units.empty())
                {
                    var.putAtt("units", varDesc.units);
                }

                if (varDesc.coordinates)
                {
                    var.putAtt("coordinates", *varDesc.coordinates);
                }

                if (varDesc.range)
                {
                    std::vector<float> range = {varDesc.range->start, varDesc.range->end};
                    var.putAtt("valid_range", getNcType<float>(), 2, range.data());
                }
            }

            obsGroups.insert({categories, file});



            // Create the groups



//
//            auto policy = ioda::detail::DataLayoutPolicy::Policies::ObsGroup;
//            auto layoutPolicy = ioda::detail::DataLayoutPolicy::generate(policy);
//            auto obsGroup = ioda::ObsGroup::generate(rootGroup, allDims, layoutPolicy);

//            auto group = nc::NcGroup();

//
//            // Create Globals
//            for (auto &global: description_.getGlobals())
//            {
//                global->addTo(rootGroup);
//            }
//
//            // Write the Dimension Variables
//            for (const auto &dimDesc: description_.getDims())
//            {
//                if (!dimDesc.source.empty())
//                {
//                    auto dataObject = dataContainer->get(dimDesc.source, categories);
//                    for (size_t dimIdx = 0; dimIdx < dataObject->getDims().size(); dimIdx++)
//                    {
//                        auto dimPath = dataObject->getDimPaths()[dimIdx];
//
//                        NamedPathDims namedPathDims;
//                        if (dimIdx == 0)
//                        {
//                            namedPathDims = namedLocDims;
//                        } else
//                        {
//                            namedPathDims = namedExtraDims;
//                        }
//
//                        auto dimName = dimForDimPath(dimPath, namedPathDims).name;
//                        auto dimVar = obsGroup.vars[dimName];
//                        dimMap[dimName]->write(dimVar);
//                    }
//                }
//            }
//
//            // Write all the other Variables
//            for (const auto &varDesc: description_.getVariables())
//            {
//                std::vector<ioda::Dimensions_t> chunks;
//                auto dimensions = std::vector<ioda::Variable>();
//                auto dataObject = dataContainer->get(varDesc.source, categories);
//                for (size_t dimIdx = 0; dimIdx < dataObject->getDims().size(); dimIdx++)
//                {
//                    auto dimPath = dataObject->getDimPaths()[dimIdx];
//
//                    NamedPathDims namedPathDims;
//                    if (dimIdx == 0)
//                    {
//                        namedPathDims = namedLocDims;
//                    } else
//                    {
//                        namedPathDims = namedExtraDims;
//                    }
//
//                    auto dimVar = obsGroup.vars[dimForDimPath(dimPath, namedPathDims).name];
//                    dimensions.push_back(dimVar);
//
//                    if (dimIdx < varDesc.chunks.size())
//                    {
//                        chunks.push_back(std::min(dimVar.getChunkSizes()[0],
//                                                  varDesc.chunks[dimIdx]));
//                    } else
//                    {
//                        chunks.push_back(dimVar.getChunkSizes()[0]);
//                    }
//                }
//
//                // Check that dateTime variable has the right dimensions
//                if (varDesc.name == "MetaData/dateTime" || varDesc.name == "MetaData/datetime")
//                {
//                    if (dimensions.size() != 1)
//                    {
//                        throw eckit::BadParameter(
//                            "IODA requires Datetime variable to be one dimensional.");
//                    }
//                }
//
//                auto var = dataObject->createVariable(obsGroup,
//                                                      varDesc.name,
//                                                      dimensions,
//                                                      chunks,
//                                                      varDesc.compressionLevel);
//
//                var.atts.add<std::string>("long_name", {varDesc.longName}, {1});
//
//                if (!varDesc.units.empty())
//                {
//                    var.atts.add<std::string>("units", {varDesc.units}, {1});
//                }
//
//                if (varDesc.coordinates)
//                {
//                    var.atts.add<std::string>("coordinates", {*varDesc.coordinates}, {1});
//                }
//
//                if (varDesc.range)
//                {
//                    var.atts.add<float>("valid_range",
//                                        {varDesc.range->start, varDesc.range->end},
//                                        {2});
//                }
//            }
//
//            obsGroups.insert({categories, obsGroup});
        }

        return obsGroups;
    }

    std::string Encoder::makeStrWithSubstitions(const std::string &prototype,
                                                const std::map<std::string,
                                                std::string> &subMap)
    {
        auto resultStr = prototype;
        auto subIdxs = findSubIdxs(prototype);

        std::reverse(subIdxs.begin(), subIdxs.end());

        for (const auto &subs: subIdxs)
        {
            if (subMap.find(subs.first) != subMap.end())
            {
                auto repIdxs = subs.second;
                resultStr = resultStr.replace(repIdxs.first,
                                              repIdxs.second - repIdxs.first + 1,
                                              subMap.at(subs.first));
            } else
            {
                std::ostringstream errStr;
                errStr << "Can't find " << subs.first << ". No category with that name.";
                throw eckit::BadParameter(errStr.str());
            }
        }

        return resultStr;
    }

    std::vector<std::pair<std::string, std::pair<int, int>>>
    Encoder::findSubIdxs(const std::string &str)
    {
        std::vector<std::pair<std::string, std::pair<int, int>>> result;

        size_t startPos = 0;
        size_t endPos = 0;

        while (startPos < str.size())
        {
            startPos = str.find("{", startPos+1);

            if (startPos < str.size())
            {
                endPos = str.find("}", startPos+1);

                if (endPos < str.size())
                {
                    result.push_back({str.substr(startPos + 1, endPos - startPos - 1),
                                      {startPos, endPos}});
                    startPos = endPos;
                }
                else
                {
                    throw eckit::BadParameter("Unmatched { found in output filename.");
                }
            }
        }

        return result;
    }

    bool Encoder::existsInNamedPath(const Query &path, const NamedPathDims &pathMap) const
    {
        for (auto &paths: pathMap)
        {
            if (std::find(paths.first.begin(), paths.first.end(), path) != paths.first.end())
            {
                return true;
            }
        }

        return false;
    }

    DimensionDescription Encoder::dimForDimPath(const Query &path,
                                                const NamedPathDims &pathMap) const
    {
        DimensionDescription dimDesc;

        for (auto paths: pathMap)
        {
            if (std::find(paths.first.begin(), paths.first.end(), path) != paths.first.end())
            {
                dimDesc = paths.second;
                break;
            }
        }

        return dimDesc;
    }


    std::pair<std::string, std::string> Encoder::splitName(std::string name) const
    {
        std::string groupName;
        std::string varName;
        size_t pos = name.find_first_of("/@");
        if (pos != std::string::npos)
        {
            if (name[pos] == '/')
            {
                groupName = name.substr(0, pos);
                varName = name.substr(pos + 1);
            }
            else
            {
                groupName = name.substr(pos + 1);
                varName = name.substr(0, pos);
            }
        }
        else
        {
            throw eckit::BadParameter("Variable name must contain a group name");
        }

        return std::make_pair(groupName, varName);
    }
}  // namespace netcdf
}  // namespace encoders
}  // namespace bufr
