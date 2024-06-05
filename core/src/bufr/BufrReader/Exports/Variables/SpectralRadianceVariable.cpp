// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "SpectralRadianceVariable.h"

#include <time.h>

#include <cmath>
#include <iomanip>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "bufr/DataObject.h"
#include "../../../DataObjectBuilder.h"
#include "eckit/exception/Exceptions.h"

namespace
{
    namespace ConfKeys
    {
        const char* SensorChannelNumber = "sensorChannelNumber";
        const char* StartChannel = "startChannel";
        const char* EndChannel = "endChannel";
        const char* ScaleFactor = "scaleFactor";
        const char* ScaledSpectralRadiance = "scaledSpectralRadiance";
    }  // namespace ConfKeys

    const std::vector<std::string> FieldNames = {ConfKeys::SensorChannelNumber,
                                                 ConfKeys::StartChannel,
                                                 ConfKeys::EndChannel,
                                                 ConfKeys::ScaleFactor,
                                                 ConfKeys::ScaledSpectralRadiance};
}  // namespace


namespace bufr {
    SpectralRadianceVariable::SpectralRadianceVariable(const std::string& exportName,
                                                       const std::string& groupByField,
                                                       const eckit::LocalConfiguration &conf) :
      Variable(exportName, groupByField, conf)
    {
        initQueryMap();
    }

    std::shared_ptr<DataObjectBase> SpectralRadianceVariable::exportData(const BufrDataMap& map)
    {
        checkKeys(map);

        // Read the variables from the map
        auto& radObj = map.at(getExportKey(ConfKeys::ScaledSpectralRadiance));
        auto& sensorChanObj = map.at(getExportKey(ConfKeys::SensorChannelNumber));
        auto& startChanObj = map.at(getExportKey(ConfKeys::StartChannel));
        auto& endChanObj = map.at(getExportKey(ConfKeys::EndChannel));
        auto& scaleFactorObj = map.at(getExportKey(ConfKeys::ScaleFactor));

        // Declare unscale spectral radiance data array from scaled spectral radiance
        std::vector<float> outData((radObj->size()), DataObject<float>::missingValue());

        // Get dimensions
        size_t nchns = (radObj->getDims())[1];
        size_t nbands = (startChanObj->getDims())[1];

        // Convert the scaled radiance to unscaled radiance
        for (size_t idx = 0; idx < radObj->size(); idx++)
        {
            auto channel = sensorChanObj->getAsInt(idx);
            size_t iloc = static_cast<size_t>(floor(idx / nchns));
            size_t bandOffset;

            for (size_t ibnd = 0; ibnd < nbands; ibnd++)
            {
                bandOffset = iloc * nbands + ibnd;
               if (channel >= startChanObj->getAsInt(bandOffset) &&
                   channel <= endChanObj->getAsInt(bandOffset)) break;
            }

            if (!radObj->isMissing(idx) && !scaleFactorObj->isMissing(bandOffset))
            {
                auto scaleFactor = powf(10.0f, -(scaleFactorObj->getAsFloat(bandOffset) ));
                outData[idx] = radObj->getAsFloat(idx) * scaleFactor;
            }
        }

//        return std::make_shared<DataObject<float>>(outData,
//                                                   getExportName(),
//                                                   groupByField_,
//                                                   radObj->getDims(),
//                                                   radObj->getPath(),
//                                                   radObj->getDimPaths());

        return DataObjectBuilder::make<float>(outData,
                                              getExportName(),
                                              groupByField_,
                                              radObj->getDims(),
                                              radObj->getPath(),
                                              radObj->getDimPaths());

//        DataObject make(const std::vector<T>& data,
//                           const std::string& fieldName,
//                           const std::string& groupByFieldName,
//                           const std::vector<int>& dims,
//                           const std::string& query,
//                           const std::vector<Query>& dimPaths

//        return DataObjectBuilder::make<float>(outData,
//                                             getExportName(),
//                                             groupByField_,
//                                             radObj->getDims(),
//                                             radObj->getPath(),
//                                             radObj->getDimPaths());
    }

    void SpectralRadianceVariable::checkKeys(const BufrDataMap& map)
    {
        std::vector<std::string> requiredKeys;
        for (const auto& fieldName : FieldNames)
        {
            if (conf_.has(fieldName))
            {
                requiredKeys.push_back(getExportKey(fieldName));
            }
        }

        std::stringstream errStr;
        errStr << "Query ";

        bool isKeyMissing = false;
        for (const auto& key : requiredKeys)
        {
            if (map.find(key) == map.end())
            {
                isKeyMissing = true;
                errStr << key;
                break;
            }
        }

        errStr << " could not be found during export of datetime object.";

        if (isKeyMissing)
        {
            throw eckit::BadParameter(errStr.str());
        }
    }

    QueryList SpectralRadianceVariable::makeQueryList() const
    {
        auto queries = QueryList();

        for (const auto& fieldName : FieldNames)
        {
            if (conf_.has(fieldName))
            {
                QueryInfo info;
                info.name = getExportKey(fieldName);
                info.query = conf_.getString(fieldName);
                info.groupByField = groupByField_;
                queries.push_back(info);
            }
        }

        return queries;
    }

    std::string SpectralRadianceVariable::getExportKey(const std::string& name) const
    {
        return getExportName() + "_" + name;
    }
}  // namespace bufr
