// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "SensorScanAngleVariable.h"

#include <cmath>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "bufr/DataObject.h"
#include "../../../DataObjectBuilder.h"
#include "eckit/exception/Exceptions.h"

namespace
{
    // Configuration keys used in LocalConfiguration
    namespace ConfKeys
    {
        const char* FieldOfViewNumber = "fieldOfViewNumber";
        const char* FieldOfRegardNumber = "fieldOfRegardNumber"; // optional
        const char* ScanStart = "scanStart";
        const char* ScanStep = "scanStep";
        const char* ScanStepAdjust = "scanStepAdjust"; // optional 
        const char* Sensor = "sensor";
    }  // namespace ConfKeys

    // Ordered list of fields to query
    const std::vector<std::string> FieldNames = {ConfKeys::FieldOfViewNumber, ConfKeys::FieldOfRegardNumber};

    // CrIS-specific FOV geometry
    static const std::array<float, 9> FovDist = {
        2.71510e-2f, 1.91986e-2f, 2.71510e-2f,
        1.91986e-2f, 0.0f,        1.91986e-2f,
        2.71510e-2f, 1.91986e-2f, 2.71510e-2f
    }; // unit is radians

    static const std::array<float, 9> FovAng = {
        4.77057f, 3.98517f, 3.19977f,
        5.55597f, 0.0f,     2.41437f,
        0.05818f, 0.84358f, 1.62897f
    }; // unit is radians

    // Degrees to radians conversion
    static float degToRad(float degrees) { return degrees * (M_PI / 180.0f); }

    // Radians to degree conversion
    static float radToDeg(float radians) { return radians * (180.0f / M_PI); }

}  // namespace


namespace bufr {

    /// Constructor: initializes base class and query map 
    SensorScanAngleVariable::SensorScanAngleVariable(const std::string& exportName,
                                                     const std::string& groupByField,
                                                     const eckit::LocalConfiguration &conf) :
      Variable(exportName, groupByField, conf)
    {
        initQueryMap();
    }

    // Export Data 
    std::shared_ptr<DataObjectBase> SensorScanAngleVariable::exportData(const BufrDataMap& map)
    {
        checkKeys(map);

        // Required parameters
        if (!conf_.has(ConfKeys::Sensor) || !conf_.has(ConfKeys::ScanStart) || !conf_.has(ConfKeys::ScanStep)) {
            throw eckit::BadParameter("Missing required parameters: sensor, scanStart and scanStep are required. Check configuration.");
        }

        // Extract required parameters
        const std::string sensor = conf_.getString(ConfKeys::Sensor);
        const float start = conf_.getFloat(ConfKeys::ScanStart);
        const float step  = conf_.getFloat(ConfKeys::ScanStep);
        const float stepAdj = conf_.has(ConfKeys::ScanStepAdjust)
                            ? conf_.getFloat(ConfKeys::ScanStepAdjust) : 0.0f;

        auto& fovnObj = map.at(getExportKey("fieldOfViewNumber"));
        const size_t nobs = fovnObj->size();
        auto fovn = std::dynamic_pointer_cast<DataObject<int>>(fovnObj)->getRawData();
        std::vector<float> scanang(nobs, DataObject<float>::missingValue());

        // Get required data for CrIS
        std::vector<int> fornVec;
        std::vector<int>* fornPtr = nullptr;
        if (sensor == "cris" && conf_.has(ConfKeys::FieldOfRegardNumber)) {
            auto& fornObj = map.at(getExportKey("fieldOfRegardNumber"));
            fornVec = std::dynamic_pointer_cast<DataObject<int>>(fornObj)->getRawData();
            fornPtr = &fornVec;
        }

        // Dispatch to sensor-specific function
        static const std::unordered_map<std::string, ComputeFn> dispatch = {
            { "iasi", computeIasi },
            { "cris", computeCris }
        };

        auto it = dispatch.find(sensor);
        if (it != dispatch.end()) {
            it->second(scanang, fovn, fornPtr, start, step, stepAdj);
        } else {
            computeGeneric(scanang, fovn, nullptr, start, step, stepAdj);
        }

        // Export scan angle
        return DataObjectBuilder::make<float>(
            scanang,
            getExportName(),
            groupByField_,
            fovnObj->getDims(),
            fovnObj->getPath(),
            fovnObj->getDimPaths()
        );
    }

    /// Validate presence of required keys in map
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

    /// Build query list for required BUFR fields
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

    /// Build export key for a mnemonic
    std::string SensorScanAngleVariable::getExportKey(const std::string& name) const
    {
        return getExportName() + "_" + name;
    }

    /// Compute IASI scan angles
    void SensorScanAngleVariable::computeIasi(std::vector<float>& scanang,
                                              const std::vector<int>& fovn,
                                              const std::vector<int>*,
                                              float start, float step, float stepAdj)
    {
        for (size_t i = 0; i < fovn.size(); ++i) {
            int scanpos = (fovn[i] - 1) / 2 + 1;
            float offset = (scanpos % 2 == 1) ? stepAdj : -stepAdj;
            scanang[i] = start + ((fovn[i] - 1) / 4) * step + offset;
        }
    }

    /// Compute CrIS scan angles
    void SensorScanAngleVariable::computeCris(std::vector<float>& scanang,
                                              const std::vector<int>& fovn,
                                              const std::vector<int>* forn,
                                              float start, float step, float)
    {
        if (!forn) throw eckit::BadParameter("CrIS scan angle requires fieldOfRegardNumber");

        for (size_t i = 0; i < fovn.size(); ++i) {
            int forIdx = (*forn)[i] - 1;
            int fovIdx = fovn[i] - 1;
            float offset = static_cast<float>(forIdx) * step;
            float scanRad = degToRad(start + offset);
            float twistRad = degToRad(FovAng[fovIdx] - offset);
            scanRad += FovDist[fovIdx] * sinf(twistRad);
            scanang[i] = radToDeg(scanRad);
        }
    }
  
    /// Generic scan angle computation (linear scan pattern)
    void SensorScanAngleVariable::computeGeneric(std::vector<float>& scanang,
                                                 const std::vector<int>& fovn,
                                                 const std::vector<int>*,
                                                 float start, float step, float)
    {
        for (size_t i = 0; i < fovn.size(); ++i) {
            scanang[i] = start + static_cast<float>(fovn[i] - 1) * step;
        }
    }

}  // namespace bufr
