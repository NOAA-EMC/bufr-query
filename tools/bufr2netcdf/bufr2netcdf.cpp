/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#include <string>
#include <iostream>
#include <ostream>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

#include "bufr/BufrParser.h"
#include "bufr/encoders/Description.h"
#include "bufr/encoders/netcdf/Encoder.h"

namespace bufr {
//    typedef ObjectFactory<Parser, const eckit::LocalConfiguration&> ParseFactory;

    std::string makeFilename(const std::string& prototype, const SubCategory& categories)
    {
        if (categories.empty())
        {
            return prototype;
        }

        // take a string formatted like ex: abc/gdas.{{ category }}.nc and replace {{ category }}
        // with the category string. The category string is the concatenation of the subcategories
        // with an underscore.

        std::string filename = prototype;
        std::string category = "";
        for (const auto& subCat : categories)
        {
            category += subCat + "_";
        }
        category.pop_back();

        size_t startPos = filename.find("{{");
        size_t endPos = filename.find("}}") + 2;
        if (startPos != std::string::npos)
        {
            filename.replace(startPos, endPos-startPos, category);
        }
        return filename;
    }

    void parse(const std::string& obsFile,
               const std::string& mappingFile,
               const std::string& outputFile,
               const std::string& tablePath = "",
               std::size_t numMsgs = 0)
    {
        std::unique_ptr<eckit::YAMLConfiguration>
            yaml(new eckit::YAMLConfiguration(eckit::PathName(mappingFile)));

        if (yaml->has("encoder"))
        {
            auto data = BufrParser(obsFile,
                                   yaml->getSubConfiguration("bufr"), tablePath).parse(numMsgs);

            auto backend = encoders::netcdf::Encoder::Backend(false, outputFile);

            auto encoderConf = yaml->getSubConfiguration("encoder");
            encoders::netcdf::Encoder(encoderConf).encode(data, backend);
        }
        else
        {
            eckit::BadParameter("No section named \"observations\"");
        }
    }
}  // namespace bufr


static void showHelp()
{
    std::cerr << "Usage: bufr2netcdf.x [-t TABLE_PATH]  [-n NUM_MESSAGES] SRC_FILE MAPPING_FILE"
              << " OUT_FILE\n"
              << "Options:\n"
              << "  -h,  Show this help message\n"
              << "  -t TABLE_PATH,  Path to BUFR table files (use with WMO BUFR files)\n"
              << "  -n NUM_MESSAGES,  Number of BUFR messages to parse.\n"
              << "Example:\n"
              << "  bufr2netcdf.x input/mhs.bufr input/mhs_mapping.yaml output/mhs.nc\n"
              << std::endl;
}


int main(int argc, char **argv)
{
    if (argc < 4)
    {
        showHelp();
        return 0;
    }

    std::string obsFile;
    std::string mappingFile;
    std::string outputFile;
    std::string tablePath = "";
    std::size_t numMsgs = 0;

    enum class ReqArgType
    {
        ObsFile = 0,
        MappingFile = 1,
        OutputFile = 2
    };

    auto reqArgIdx = ReqArgType::ObsFile;
    std::size_t argIdx = 1;
    while (argIdx < static_cast<std::size_t> (argc))
    {
        if (strcmp(argv[argIdx], "-n") == 0)
        {
            if (static_cast<std::size_t> (argc) > argIdx + 1)
            {
                numMsgs = atoi(argv[argIdx + 1]);
            } else
            {
                showHelp();
                return 0;
            }

            argIdx += 2;
        } else if (strcmp(argv[argIdx], "-h") == 0)
        {
            showHelp();
            return 0;
        } else if (strcmp(argv[argIdx], "-t") == 0)
        {
            if (static_cast<std::size_t> (argc) > argIdx + 1)
            {
                tablePath = std::string(argv[argIdx + 1]);
            } else
            {
                showHelp();
                return 0;
            }

            argIdx += 2;
        } else
        {
            switch (reqArgIdx)
            {
                case ReqArgType::ObsFile:
                    obsFile = std::string(argv[argIdx]);
                    break;
                case ReqArgType::MappingFile:
                    mappingFile = std::string(argv[argIdx]);
                    break;
                case ReqArgType::OutputFile:
                    outputFile = std::string(argv[argIdx]);
                    break;
                default:
                    break;
            }

            argIdx++;
            reqArgIdx = static_cast<ReqArgType> (static_cast<int> (reqArgIdx) + 1);
        }
    }

    std::cout << "outputFile: " << outputFile << std::endl;

    bufr::parse(obsFile, mappingFile, outputFile, tablePath, numMsgs);

    return 0;
}  // namespace bufr
