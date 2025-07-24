// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>

#include "eckit/config/LocalConfiguration.h"

#include "bufr/Variable.h"


namespace bufr {

    /// \brief Function type for computing scan angles
    using ComputeFn = std::function<std::vector<float>(const eckit::LocalConfiguration& conf,
		                                       const BufrDataMap& map)>;

    /// \brief Exports parsed data as SensorScanAngle  using speciefied Mnemonics
    class SensorScanAngleVariable final : public Variable
    {
     public:
        SensorScanAngleVariable() = delete;
        SensorScanAngleVariable(const std::string& exportName,
                                const std::string& groupByField,
                                const eckit::LocalConfiguration& conf);

        ~SensorScanAngleVariable() final = default;

        /// \brief Get the configured mnemonics and turn them into SensorScanAngle
        /// \param map BufrDataMap that contains the parsed data for each mnemonic
        std::shared_ptr<DataObjectBase> exportData(const BufrDataMap& map) final;

        /// \brief Get a list of queries for this variable
        QueryList makeQueryList() const final;

     private:
        /// \brief makes sure the bufr data map has all the required keys.
        void checkKeys(const BufrDataMap& map);

        /// \brief get the export key string
        std::string getExportKey(const std::string& name) const;

	/// \brief Compute scan angle for IASI instrument
        static std::vector<float> computeIasi(const eckit::LocalConfiguration& conf,
		                      	      const BufrDataMap& map);

        /// \brief Compute scan angle for CrIS instrument
        static std::vector<float> computeCris(const eckit::LocalConfiguration& conf,
		                  	      const BufrDataMap& map);

        /// \brief Compute scan angle using generic (default) logic
        static std::vector<float> computeGeneric(const eckit::LocalConfiguration& conf,
		                        	 const BufrDataMap& map);
    };
}  // namespace bufr
