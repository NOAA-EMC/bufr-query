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
    namespace ConfKeys
    {
        const char* FieldOfViewNumber = "fieldOfViewNumber";
        const char* FieldOfRegardNumber = "fieldOfRegardNumber"; // optional
        const char* ScanStart = "scanStart";
        const char* ScanStep = "scanStep";
        const char* ScanStepAdjust = "scanStepAdjust"; // optinal 
        const char* Sensor = "sensor";
    }  // namespace ConfKeys

    const std::vector<std::string> FieldNames = {ConfKeys::FieldOfViewNumber };

    static const std::array<float, 9> fov_dist = {
        2.71510e-2f, 1.91986e-2f, 2.71510e-2f,
        1.91986e-2f, 0.0f,        1.91986e-2f,
        2.71510e-2f, 1.91986e-2f, 2.71510e-2f
    }; // unit is radians
    static const std::array<float, 9> fov_ang = {
        4.77057f, 3.98517f, 3.19977f,
        5.55597f, 0.0f,     2.41437f,
        0.05818f, 0.84358f, 1.62897f
    }; // unit is radians

    // Degrees to radians conversion
    constexpr float degToRad(float degrees) {
        return degrees * (M_PI / 180.0f);
    }

    // Radians to degree conversion
    constexpr float radToDeg(float radians) {
        return radians * (180.0f / M_PI);
    }

    // Compute scan angle for IASI 
    void computeIasiScanAngles(std::vector<float>& scanang, const std::vector<int>& fovn,
                               const std::vector<int>& scanpos, float start, float step, float stepAdj)
    {
        float tmp = -stepAdj;
        for (size_t idx = 0; idx < fovn.size(); ++idx)
       	{
            if (scanpos[idx] % 2 == 1) tmp = stepAdj;
            scanang[idx] = start + static_cast<float>((fovn[idx] - 1) / 4) * step + tmp;
        }
    }

    // Compute scan angle for CrIS, including FOR-dependent twist correction
    void computeCrisScanAngles(std::vector<float>& scanang, const std::vector<int>& fovn,
                               const std::vector<int>& forn, float start, float step)
    {
 
       for (size_t idx = 0; idx < fovn.size(); ++idx)
       {
           const int for_idx = forn[idx] - 1;    // 0-based index
           const int fov_idx = fovn[idx] - 1;    // 0-based index

           // Degrees → Radians
           float angle_offset_deg = static_cast<float>(for_idx) * step;
           float scan_angle_rad = degToRad(start + angle_offset_deg);
           float twist_angle_rad = degToRad(fov_ang[fov_idx] - angle_offset_deg);

           // Add twist component
           scan_angle_rad += fov_dist[fov_idx] * sinf(twist_angle_rad);

           // Convert to degrees if final output should be in degrees
           scanang[idx] = radToDeg(scan_angle_rad);
       } 
    }

    // Compute scan angle based on fovn, start angle and angle increment (default) 
    void computeGenericScanAngles(std::vector<float>& scanang, const std::vector<int>& fovn,
                                  float start, float step)
    {
        for (size_t idx = 0; idx < fovn.size(); ++idx)
       	{
            scanang[idx] = start + static_cast<float>(fovn[idx] - 1) * step;
        }
    }
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

        // Sensor string is required
        const std::string sensor = conf_.getString(ConfKeys::Sensor);

        // Required parameters
        if (!conf_.has(ConfKeys::ScanStart) || !conf_.has(ConfKeys::ScanStep)) {
            throw eckit::BadParameter("Missing required parameters: scanStart or scanStep. Check configuration.");
        }
        const float start = conf_.getFloat(ConfKeys::ScanStart);
        const float step  = conf_.getFloat(ConfKeys::ScanStep);

        // Optional: only needed for IASI
        float stepAdj = 0.0f;
        if (sensor == "iasi") {
            if (!conf_.has(ConfKeys::ScanStepAdjust)) {
                throw eckit::BadParameter("Missing required parameter: scanStepAdjust for IASI");
            }
            stepAdj = conf_.getFloat(ConfKeys::ScanStepAdjust);
        }

        // Get required input field: FOV number
        auto& fovnObj = map.at(getExportKey(ConfKeys::FieldOfViewNumber));
        const size_t nobs = fovnObj->size();
        auto fovn = std::dynamic_pointer_cast<DataObject<int>>(fovnObj)->getRawData();

        // Declare and initialize scanline array
        // scanline has the same dimension as fovn
        std::vector<float> scanang(nobs, DataObject<float>::missingValue());
        std::vector<int> scanpos(nobs);
        std::vector<int> forn;

        if (sensor == "cris")
        {
           const auto& fornObj = map.at(getExportKey(ConfKeys::FieldOfRegardNumber));
           forn = std::dynamic_pointer_cast<DataObject<int>>(fornObj)->getRawData();
	}

        // Compute scan position (1-based)
        if (sensor == "iasi") {
            for (size_t idx = 0; idx < nobs; ++idx) {
                scanpos[idx] = (fovnObj->getAsInt(idx) - 1) / 2 + 1;
            }
        } else {
            scanpos = std::dynamic_pointer_cast<DataObject<int>>(fovnObj)->getRawData();
        }

        // Dispatch to scan angle calculation
        if (sensor == "iasi") {
            computeIasiScanAngles(scanang, fovn, scanpos, start, step, stepAdj);
        } else if (sensor == "cris") {
            computeCrisScanAngles(scanang, fovn, forn, start, step);
        } else {
            computeGenericScanAngles(scanang, fovn, start, step);
        }

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
