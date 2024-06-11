// (C) Copyright 2020-2024 NOAA/NWS/NCEP/EMC

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <mpi.h>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/mpi/Comm.h"


#include "File.h"
#include "BufrTypes.h"
#include "BufrDescription.h"
#include "DataContainer.h"


namespace bufr {
    /// \brief Uses a BufrDescription and helper classes to parse the contents of a BUFR file.
    class BufrParser
    {
     public:
       BufrParser(const std::string& obsfile,
                  const BufrDescription& description,
                  const std::string& tablepath = "");

       BufrParser(const std::string& obsfile,
                  const eckit::LocalConfiguration& conf,
                  const std::string& tablepath = "");

       BufrParser(const std::string& obsfile,
                  const std::string& mappingPath,
                  const std::string& tablepath = "");

        ~BufrParser();

        /// \brief Uses the provided description to parse the buffer file.
        /// \param maxMsgsToParse Messages to parse (0 for everything)
        std::shared_ptr<DataContainer> parse(const size_t maxMsgsToParse = 0);

        /// \brief Uses the provided description to parse the buffer file using MPI.
        /// \param comm The eckit MPI comm object
        std::shared_ptr<DataContainer> mpiParse(const eckit::mpi::Comm&);

        /// \brief Start over from beginning of the BUFR file
        void reset();

     private:
        typedef std::map<std::vector<std::string>, BufrDataMap> CatDataMap;

        /// \brief The description the defines what to parse from the BUFR file
        BufrDescription description_;

        /// \brief The Bufr file object we are working with
        File file_;

        /// \brief Exports collected data into a DataContainer
        /// \param srcData Data to export
        std::shared_ptr<DataContainer> exportData(const BufrDataMap& srcData);

        /// \brief Function responsible for dividing the data into subcategories.
        /// \details This function is intended to be called over and over for each specified Split
        ///          object, sub-splitting the data given into all the possible subcategories.
        /// \param splitMaps Pre-split map of data.
        /// \param split Object that knows how to split data.
        CatDataMap splitData(CatDataMap& splitMaps, Split& split);

        /// \brief Opens a BUFR file using the Fortran BUFR interface.
        /// \param filepath Path to bufr file.
        /// \param isWmoFormat _optional_ Bufr file is in the standard format.
        /// \param tablepath _optional_ Path to WMO master tables (needed for standard bufr files).

        /// \brief Convenience method to print the Categorical data map to stdout.
        void printMap(const CatDataMap& map);
    };
}  // namespace bufr
