// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <string>

#include "ResultSet.h"
#include "QuerySet.h"
#include "DataProvider.h"

namespace bufr {

    /// \brief Manages an open BUFR file.
    class File
    {
     public:
        File() = delete;

        File(const std::string& filename,
             const std::string& wmoTablePath = "");

        /// \brief Execute the queries given in the query set over the BUFR file and accumulate the
        /// resulting data in the ResultSet.
        /// \param query_set The queryset object that contains the collection of desired queries
        /// \param offset The index of the message in the file to start reading from
        /// \param numMessages The number of messages to read from the file
        ResultSet execute(const QuerySet& query_set,
                          size_t offset = 0,
                          size_t numMessages = 0);

        /// \brief Number of messages in the currently open file..
        size_t size(const QuerySet& querySet = QuerySet());

        /// \brief Close the currently opened BUFR file.
        void close();

        /// \brief Rewind the currently opened BUFR file to the beginning.
        void rewind();

     private:
        std::shared_ptr<DataProvider> dataProvider_;
    };
}  // namespace bufr
