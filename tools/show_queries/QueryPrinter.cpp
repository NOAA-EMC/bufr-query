// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "QueryPrinter.h"

#include <algorithm>
#include <iostream>
#include <map>


namespace bufr {

  QueryPrinter::QueryPrinter(DataProviderType dataProvider) : dataProvider_(dataProvider) {}

  void QueryPrinter::printQueries(const std::string& subset) {
    if (!subset.empty()) {
      auto table = getTable({subset, 0});
      std::cout << subset << std::endl;
      std::cout << " Dimensioning Sub-paths: " << std::endl;
      printDimPaths(getDimPaths(table));
      std::cout << std::endl;
      std::cout << " Queries: " << std::endl;
      printQueryList(table);
      std::cout << std::endl;
    } else {
      dataProvider_->initAllTableData();
      auto variants = getSubsetVariants();

      if (variants.empty()) {
        std::cerr << "No BUFR variants found in " << dataProvider_->getFilepath() << std::endl;
        exit(1);
      }

      std::cout << "Available subset variants: " << std::endl;
      for (auto v : variants) {
        std::cout << v.str() << std::endl;
      }

      std::cout << "Total number of subset variants found: " << variants.size() << std::endl
                << std::endl;

      for (const auto& v : variants) {
        auto queries = getTable(v);

        std::cout << v.str() << std::endl;
        std::cout << " Dimensioning Sub-paths: " << std::endl;
        printDimPaths(getDimPaths(queries));
        std::cout << std::endl;
        std::cout << " Queries: " << std::endl;
        printQueryList(queries);
        std::cout << std::endl;
      }
    }
  }

  std::vector<std::pair<int, std::string>> QueryPrinter::getDimPaths(const SubsetTableType& table) {
    std::map<std::string, std::pair<int, std::string>> dimPathMap;
    for (const auto& leaf : table->getLeaves()) {
      for (const auto& path : leaf->getDimPaths()) {
        dimPathMap[path] = std::make_pair(leaf->getDimIdxs().size(), path);
      }
    }

    std::vector<std::pair<int, std::string>> result;
    for (auto& dimPath : dimPathMap) {
      result.push_back(dimPath.second);
    }

    return result;
  }

  std::string QueryPrinter::dimStyledStr(int dims) {
    std::ostringstream ostr;
    ostr << dims << "d";

    return ostr.str();
  }

  std::string QueryPrinter::typeStyledStr(const TypeInfo& info) {
    std::string typeStr;

    if (info.isString()) {
      typeStr = "string";
    } else if (info.isInteger()) {
      if (info.isSigned()) {
        if (info.is64Bit()) {
          typeStr = "int64 ";
        } else {
          typeStr = "int   ";
        }
      } else {
        if (info.is64Bit()) {
          typeStr = "uint64";
        } else {
          typeStr = "uint  ";
        }
      }
    } else {
      if (info.is64Bit()) {
        typeStr = "double";
      } else {
        typeStr = "float ";
      }
    }

    return typeStr;
  }

  std::string QueryPrinter::descStyledStr(const TypeInfo& info) {
    return info.description;
  }

  void QueryPrinter::printDimPaths(std::vector<std::pair<int, std::string>> dimPaths) {
    for (auto& dimPath : dimPaths) {
      std::cout << "  " << dimPath.first << "d  " << dimPath.second << std::endl;
    }
  }

  void QueryPrinter::printQueryList(const SubsetTableType& table) {
    std::vector<std::string> queryDescs;
    std::vector<std::string> descriptions;
    for (auto leaf : table->getLeaves()) {
      int numDims = static_cast<int>(leaf->getDimIdxs().size());
      std::vector<std::string> pathComponents;
      std::shared_ptr<BufrNode> currentNode = leaf;

      while (currentNode != nullptr) {
        if (currentNode->isQueryPathNode() || currentNode->isLeaf()) {
          std::ostringstream pathStr;
          pathStr << currentNode->mnemonic;

          if (currentNode->hasDuplicates) {
            pathStr << "[" << currentNode->copyIdx << "]";
          }

          if (currentNode->isStack())
          {
            pathStr << "{1}";
            numDims--;
          }

          pathComponents.push_back(pathStr.str());
        }

        currentNode = currentNode->parent.lock();
      }

      std::ostringstream ostr;
      ostr << dimStyledStr(numDims) << "  ";
      ostr << typeStyledStr(leaf->typeInfo) << "  ";

      for (auto it = pathComponents.rbegin(); it != pathComponents.rend(); ++it) {
        if (it != pathComponents.rbegin()) ostr << "/";
        ostr << *it;
      }

      queryDescs.push_back(ostr.str());
      descriptions.push_back(descStyledStr(leaf->typeInfo));
    }

    // Measure the longest query description
    auto maxLen = std::max_element(queryDescs.begin(), queryDescs.end(),
                                       [](const std::string& a, const std::string& b) {
                                     return a.length() < b.length();
                                       })->length();

    // Print description
    for (size_t i = 0; i < queryDescs.size(); ++i)
    {
      std::cout << queryDescs[i];
      for (size_t j = 0; j < maxLen - queryDescs[i].length(); ++j)
      {
            std::cout << " ";
      }
      std::cout << "  " << descriptions[i] << std::endl;
    }
  }

}  // namespace bufr