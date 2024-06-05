// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include "QueryPrinter.h"

#include <string>
#include <vector>


namespace bufr {

    class NcepQueryPrinter : public QueryPrinter
    {
     public:
        explicit NcepQueryPrinter(const std::string& filepath);

        /// \brief Get the query data for a specific subset variant type
        /// \param variant The subset variant
        /// \returns Vector of SubsetTable BufrNode objects
        SubsetTableType getTable(const SubsetVariant& variant) final;

        /// \brief Get a complete set of subset variants in the data file. WARNING: using this will
        ///        be slow and reset the file pointer.
        /// \returns Vector of subset variants
        std::set<SubsetVariant> getSubsetVariants() const final;
    };

}  // namespace bufr
