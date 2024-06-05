// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include "QueryPrinter.h"


namespace bufr {

    class QueryData;

    class WmoQueryPrinter : public QueryPrinter
    {
     public:
       WmoQueryPrinter(const std::string& filepath, const std::string& tablepath);
       ~WmoQueryPrinter() = default;

        /// \brief Get the query data for a specific subset variant type
        /// \param variant The subset variant
        /// \returns Vector of BufrNode QueryData objects
        SubsetTableType getTable(const SubsetVariant& variant) final;

        /// \brief Get a complete set of subsets in the data file. WARNING: using this will be slow
        ///        and reset the file pointer.
        /// \returns Vector of subset variants
        std::set<SubsetVariant> getSubsetVariants() const final;
    };

}  // namespace bufr
