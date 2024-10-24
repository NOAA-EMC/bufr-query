// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "SensorScanAngleVariable.h"

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
        const char* FieldOfViewNumber = "fieldOfViewNumber";
        const char* ScanStart = "scanStart";
        const char* ScanStep = "scanStep";
        const char* ScanStepAdjust = "scanStepAdjust";
        const char* Sensor = "sensor";
    }  // namespace ConfKeys

    const std::vector<std::string> FieldNames = {ConfKeys::FieldOfViewNumber };
}  // namespace


namespace bufr {
    SensorScanAngleVariable::SensorScanAngleVariable(const std::string& exportName,
                                                     const std::string& groupByField,
                                                     const eckit::LocalConfiguration &conf) :
      Variable(exportName, groupByField, conf)
    {
        initQueryMap();
    }

    std::shared_ptr<DataObjectBase> SensorScanAngleVariable::exportData(const BufrDataMap& map)
    {
        checkKeys(map);

        // Get input parameters for sensor scan angle calculation
        std::string sensor;
        if (conf_.has(ConfKeys::Sensor) )
        {
             sensor = conf_.getString(ConfKeys::Sensor);
        }
        else
        {
            throw eckit::BadParameter("Missing required parameters: sensor. "
                                      "Check your configuration.");
        }

        float start;
        float step;
        float stepAdj;
        if (conf_.has(ConfKeys::ScanStart) && conf_.has(ConfKeys::ScanStep))
        {
             start = conf_.getFloat(ConfKeys::ScanStart);
             step = conf_.getFloat(ConfKeys::ScanStep);
        }
        else
        {
            throw eckit::BadParameter("Missing required parameters: scan starting angle and step. "
                                      "Check your configuration.");
        }

        if (conf_.has(ConfKeys::ScanStepAdjust) && sensor == "iasi" )
        {
             sensor = conf_.getString(ConfKeys::Sensor);
             stepAdj = conf_.getFloat(ConfKeys::ScanStepAdjust);
        }

        // Read the variables from the map

        auto& fovnObj = map.at(getExportKey(ConfKeys::FieldOfViewNumber));

        // Declare and initialize scanline array
        // scanline has the same dimension as fovn
        std::vector<float> scanang(fovnObj->size(), DataObject<float>::missingValue());
        std::vector<int> scanpos(fovnObj->size(), DataObject<int>::missingValue());

        // Get field-of-view number
        std::vector<int> fovn(fovnObj->size(), DataObject<int>::missingValue());
        for (size_t idx = 0; idx < fovnObj->size(); idx++)
        {
           fovn[idx] = fovnObj->getAsInt(idx);
        }

	scan_angle = -48.3 + (3.22/2) + sln*3.22 - (1.25/2) + (fov % 2)*1.25

        return DataObjectBuilder::make<float>(scanang,
                                              getExportName(),
                                              groupByField_,
                                              fovnObj->getDims(),
                                              fovnObj->getPath(),
                                              fovnObj->getDimPaths());
    }

    void SensorScanAngleVariable::checkKeys(const BufrDataMap& map)
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

        errStr << " could not be found during export of scanang object.";

        if (isKeyMissing)
        {
            throw eckit::BadParameter(errStr.str());
        }
    }

    QueryList SensorScanAngleVariable::makeQueryList() const
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

    std::string SensorScanAngleVariable::getExportKey(const std::string& name) const
    {
        return getExportName() + "_" + name;
    }
}  // namespace bufr
