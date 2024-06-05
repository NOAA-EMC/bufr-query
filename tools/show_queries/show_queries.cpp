// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include <algorithm>
#include <iostream>
#include <string>

#include "NcepQueryPrinter.h"
#include "WmoQueryPrinter.h"


void printHelp()
{
    std::cout << "Description: " << std::endl;
    std::cout << "  Lists all the queries possible on a BUFR file per subset." << std::endl;
    std::cout << "Arguments: " << std::endl;
    std::cout << "  -h          (Optional) Print out the help message." << std::endl;
    std::cout << "  -s <subset> (Optional) Print paths only for this subset." << std::endl;
    std::cout << "  input_file  Path to the BUFR file." << std::endl;
    std::cout << "  output_file  (Optional) Save the output. " << std::endl;
    std::cout << "  -t <table_path>  (Optional) Path to the WMO table file." << std::endl;
    std::cout << "Examples: " << std::endl;
    std::cout << "  ./show_queries.x ../data/bufr_satwnd_old_format.bufr" << std::endl;
    std::cout << "  ./show_queries.x -s NC005066 ../data/bufr_satwnd_old_format.bufr" << std::endl;
}

int main(int argc, char** argv)
{
    std::string inputFile = "";
    std::string tablePath = "";
    std::string subset = "";

    int idx = 1;
    while (idx < argc)
    {
        std::string arg = argv[idx];
        if (arg.substr(0,2) == "-s")
        {
            if (arg.size() == 2)
            {
                subset = argv[idx+1];
                idx = idx + 2;
            }
            else
            {
                subset = arg.substr(2, arg.size());
                idx++;
            }
        }
        else if (arg == "-h")
        {
            printHelp();
            exit(0);
        }
        else if (arg == "-t")
        {
            tablePath = std::string(argv[idx + 1]);
            idx = idx + 2;
        }
        else
        {
            inputFile = arg;
            idx++;
        }
    }

    if (inputFile.empty())
    {
        printHelp();
        std::cerr << "Error: no input file specified" << std::endl;
        exit(1);
    }

    try
    {
        std::shared_ptr<bufr::QueryPrinter> printer;
        if (tablePath.empty())
        {
            printer = std::make_shared<bufr::NcepQueryPrinter> (inputFile);
        }
        else
        {
            printer = std::make_shared<bufr::WmoQueryPrinter> (inputFile, tablePath);
        }

        printer->printQueries(subset);
    }
    catch (const std::exception &e)
    {
        throw;
    }

    return 0;
}


