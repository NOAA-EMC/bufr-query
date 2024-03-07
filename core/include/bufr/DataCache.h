/*
* (C) Copyright 2024 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/


#include <unordered_map>
#include <vector>
#include <string>

#include "oops/util/Logger.h"

#include "bufr/DataContainer.h"

#pragma once

namespace bufr {

  /// \brief A singleton class that holds the data cache. The cache is a map of unique keys
  /// (srcPath + mapPath) to a "CacheEntry" which contains a DataContainer and some metadata. The
  /// clients can mark a category as finished, and when all categories are finished, the entry is
  /// removed from the cache. If the application finishes running and there are still entries in the
  /// cache, a warning is printed.
  class DataCache {
    public:
      DataCache(DataCache const&) = delete;
      void operator=(DataCache const&) = delete;

      ~DataCache()
      {
        if (!cache_.empty())
        {
          oops::Log::info() << "DataCache destructor called with non-empty cache.";
        }
      }

      /// \brief Check if the cache contains an entry for the given srcPath and mapPath.
      /// \param srcPath The path to the source file.
      /// \param mapPath The path to the map file.
      /// \return True if the cache contains an entry for the given srcPath and mapPath.
      static bool has(const std::string& srcPath, const std::string& mapPath)
      {
          auto& cache = instance().cache_;
          auto key = makeKey(srcPath, mapPath);

          return cache.find(key) != cache.end();
      }

      /// \brief Get the DataContainer for the given srcPath and mapPath.
      /// \param srcPath The path to the source file.
      /// \param mapPath The path to the map file.
      static std::shared_ptr<DataContainer> get(const std::string& srcPath,
                                                const std::string& mapPath)
      {
          auto& cache = instance().cache_;
          auto key = makeKey(srcPath, mapPath);

          if (cache.find(key) == cache.end())
          {
            throw eckit::BadParameter("DataCache::get: No cache entry for key " + key);
          }

          return cache[key].data;
      }

      /// \brief Add a DataContainer to the cache.
      /// \param srcPath The path to the source file.
      /// \param mapPath The path to the map file.
      /// \param cachedCategories The categories we care to read.
      /// \param data The DataContainer to add to the cache.
      static void add(const std::string& srcPath,
                      const std::string& mapPath,
                      const std::vector<std::vector<std::string>>& cachedCategories,
                      std::shared_ptr<DataContainer> data)
      {
          auto& cache = instance().cache_;
          auto key = makeKey(srcPath, mapPath);

          if (cache.find(key) != cache.end())
          {
            return;
          }

          cache[key].data = data;
          cache[key].cachedCategories = cachedCategories;
      }

      /// \brief Mark a category as finished. Delete entry when all "cachedCategories" are finished.
      /// \param srcPath The path to the source file.
      /// \param mapPath The path to the map file.
      /// \param category The category to mark as finished.
      static void markFinished(const std::string& srcPath,
                               const std::string& mapPath,
                               const std::vector<std::string>& category)
      {
          auto& cache = instance().cache_;
          auto key = makeKey(srcPath, mapPath);

          if (cache.find(key) == cache.end())
          {
            throw eckit::BadParameter("DataCache::markFinished: No cache entry for key " + key);
          }

          if (std::find(cache[key].cachedCategories.begin(),
                        cache[key].cachedCategories.end(), category)
              == cache[key].cachedCategories.end())
          {
                oops::Log::info() << "DataCache::markFinished called with category that " \
                                     "is not cached.";
                return;
          }

          cache[key].finishedCategories.push_back(category);

          if (cache[key].finishedCategories.size() == cache[key].cachedCategories.size())
          {
            cache.erase(key);
          }
      }

    private:

      /// \brief Get the singleton instance of the DataCache.
      static DataCache& instance()
      {
        static DataCache instance;
        return instance;
      }

      typedef std::string Key;

      struct CacheEntry {
        std::vector<std::vector<std::string>> cachedCategories;
        std::vector<std::vector<std::string>> finishedCategories;
        std::shared_ptr<DataContainer> data;
      };

      DataCache() {}

      std::unordered_map<Key, CacheEntry> cache_;

      /// \brief Make a unique key from the given srcPath and mapPath.
      static std::string makeKey(const std::string& srcPath, const std::string& mapPath)
      {
        return srcPath + " " + mapPath;
      }
  };

}  // namespace bufr
