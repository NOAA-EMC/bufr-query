/*
* (C) Copyright 2022 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#pragma once

#include <unordered_map>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <memory>

#include "bufr/QueryParser.h"

namespace bufr {
  class QuerySetImpl;
  typedef std::set<std::string> Subsets;

  /// \brief Manages a collection of queries.
  class QuerySet
  {
  public:
    QuerySet();
    QuerySet(const QuerySet&);
    QuerySet(QuerySet&&);
    explicit QuerySet(const std::vector<std::string>& subsets);
    ~QuerySet();

    QuerySet& operator=(const QuerySet&);
    QuerySet& operator=(QuerySet&&);

    /// \brief Add a new query to the collection.
    /// \param[in] name The name of the query.
    /// \param[in] query The query string.
    void add(const std::string& name, const std::string& query);

    /// \brief Returns the size of the collection.
    size_t size() const;

    /// \brief Returns the names of all the queries.
    /// \return A vector of the names of all the queries.
    std::vector<std::string> names() const;

    /// \brief Returns a list of subsets.
    /// \return A vector of the names of all the queries.
    bool includesSubset(const std::string& subset) const;

    std::vector<Query> queriesFor(const std::string& name) const;

    friend class QueryRunner;

   private:
     std::unique_ptr<QuerySetImpl> impl_;
  };
}  // namespace bufr

