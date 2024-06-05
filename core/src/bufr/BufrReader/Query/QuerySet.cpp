// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include <iostream>
#include "bufr/QuerySet.h"

#include "QuerySetImpl.h"


namespace bufr {
  typedef std::set<std::string> Subsets;

  QuerySet::QuerySet() : impl_(std::make_unique<QuerySetImpl>()) {}

  QuerySet::QuerySet(const QuerySet& other) : impl_(std::make_unique<QuerySetImpl>(*other.impl_)) {}

  QuerySet::QuerySet(QuerySet&& other) : impl_(std::move(other.impl_)) {}

  QuerySet::QuerySet(const std::vector<std::string>& subsets) :
    impl_(std::make_unique<QuerySetImpl>(subsets)) {}

  QuerySet::~QuerySet() = default;

  QuerySet& QuerySet::operator=(const QuerySet& other)
  {
    impl_ = std::make_unique<QuerySetImpl>(*other.impl_);
    return *this;
  }

  QuerySet& QuerySet::operator=(QuerySet&&) = default;

  void QuerySet::add(const std::string& name, const std::string& query)
  {
    impl_->add(name, query);
  }

  size_t QuerySet::size() const
  {
    return impl_->size();
  }

  std::vector<std::string> QuerySet::names() const
  {
    return impl_->names();
  }

  bool QuerySet::includesSubset(const std::string& subset) const
  {
    return impl_->includesSubset(subset);
  }

  std::vector<Query> QuerySet::queriesFor(const std::string& name) const
  {
    return impl_->queriesFor(name);
  }
}  // namespace bufr
