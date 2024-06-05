// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include <algorithm>

#include "QuerySetImpl.h"

namespace bufr {
  QuerySetImpl::QuerySetImpl() :
        includesAllSubsets_(true),
        addHasBeenCalled_(false),
        limitSubsets_({}),
        presentSubsets_({})
    {
    }

  QuerySetImpl::QuerySetImpl(const std::vector<std::string>& subsets) :
        includesAllSubsets_(false),
        addHasBeenCalled_(false),
        limitSubsets_(std::set<std::string>(subsets.begin(),
                                            subsets.end())),
        presentSubsets_({})
    {
        if (limitSubsets_.empty())
        {
            includesAllSubsets_ = true;
        }
    }

    QuerySetImpl::QuerySetImpl(const QuerySetImpl& other) = default;
    QuerySetImpl::QuerySetImpl(QuerySetImpl&& other) = default;
    QuerySetImpl::~QuerySetImpl() = default;

    void QuerySetImpl::add(const std::string& name, const std::string& queryStr)
    {
        if (!addHasBeenCalled_)
        {
            addHasBeenCalled_ = true;
            includesAllSubsets_ = false;
        }

        std::vector<Query> queries;
        for (const auto &query : QueryParser::parse(queryStr))
        {
            if (limitSubsets_.empty())
            {
                includesAllSubsets_ = includesAllSubsets_ | query.subset->isAnySubset;
                presentSubsets_.insert(query.subset->name);
            }
            else
            {
                if (query.subset->isAnySubset)
                {
                    presentSubsets_ = limitSubsets_;
                }
                else
                {
                    presentSubsets_.insert(query.subset->name);

                    std::vector<std::string> newSubsets;
                    std::set_intersection(limitSubsets_.begin(),
                                          limitSubsets_.end(),
                                          presentSubsets_.begin(),
                                          presentSubsets_.end(),
                                          std::back_inserter(newSubsets));

                    presentSubsets_ = std::set<std::string>(newSubsets.begin(),
                                                            newSubsets.end());
                }
            }

            queries.emplace_back(query);
        }

        queryMap_[name] = queries;
    }

    bool QuerySetImpl::includesSubset(const std::string& subset) const
    {
        bool includesSubset = true;
        if (!includesAllSubsets_)
        {
            if (queryMap_.empty())
            {
                includesSubset = (limitSubsets_.find(subset) != limitSubsets_.end());
            }
            else
            {
                includesSubset = (presentSubsets_.find(subset) != presentSubsets_.end());
            }
        }

        return includesSubset;
    }

    std::vector<std::string> QuerySetImpl::names() const
    {
        std::vector<std::string> names;
        for (auto const& query : queryMap_)
        {
            names.push_back(query.first);
        }

        return names;
    }

    std::vector<Query> QuerySetImpl::queriesFor(const std::string& name) const
    {
        return queryMap_.at(name);
    }
}  // namespace bufr
