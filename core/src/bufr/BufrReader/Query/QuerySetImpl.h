// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <unordered_map>
#include <vector>
#include <set>
#include <string>
#include <map>

#include "bufr/QueryParser.h"

namespace bufr {
    typedef std::set<std::string> Subsets;

    /// \brief Manages a collection of queries.
    class QuerySetImpl {
     public:
        QuerySetImpl();
        explicit QuerySetImpl(const std::vector<std::string>& subsets);
        QuerySetImpl(const QuerySetImpl& other);
        QuerySetImpl(QuerySetImpl&& other);
        ~QuerySetImpl();

        /// \brief Add a new query to the collection.
        /// \param[in] name The name of the query.
        /// \param[in] query The query string.
        void add(const std::string& name, const std::string& query);

        /// \brief Returns the size of the collection.
        size_t size() const { return queryMap_.size(); }

        /// \brief Returns the names of all the queries.
        /// \return A vector of the names of all the queries.
        std::vector<std::string> names() const;

        /// \brief Returns a list of subsets.
        /// \return A vector of the names of all the queries.
        bool includesSubset(const std::string& subset) const;

        /// \brief Get list of queries for query with name
        /// \param[in] name The name of the query.
        /// \return A vector of queries.
        std::vector<Query> queriesFor(const std::string& name) const;

     private:
        std::unordered_map<std::string, std::vector<Query>> queryMap_;
        bool includesAllSubsets_;
        bool addHasBeenCalled_;
        const Subsets limitSubsets_;
        Subsets presentSubsets_;
    };
}  // namespace bufr
