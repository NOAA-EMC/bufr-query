// (C) Copyright 2022 NOAA/NWS/NCEP/EMC
#include "QueryRunner.h"

#include <string>
#include <iostream>
#include <memory>

#include "../../Log.h"
#include "bufr/SubsetTable.h"
#include "VectorMath.h"
#include "SubsetLookupTable.h"
#include "ResultSetImpl.h"


namespace bufr {
    QueryRunner::QueryRunner(const QuerySet& querySet, ResultSet& resultSet,
                             const DataProviderType &dataProvider) :
        querySet_(querySet),
        resultSet_(resultSet),
        dataProvider_(dataProvider)
    {
    }

    void QueryRunner::accumulate()
    {
        auto cacheEntry = getTargets();
        auto frame = SubsetLookupTable(dataProvider_, cacheEntry.targets, cacheEntry.layout);
        resultSet_.impl_->appendFrame(frame, cacheEntry.targets);
    }

    QueryRunner::TargetCacheEntry QueryRunner::getTargets()
    {
        // Attempt to get targets from the cache
        const auto variant = dataProvider_->getSubsetVariant();
        const auto cacheIt = targetsCache_.find(variant);
        if (cacheIt != targetsCache_.end())
        {
            return cacheIt->second;
        }

        auto table = SubsetTable(dataProvider_);

        const auto targets = std::make_shared<Targets>();
        targets->reserve(querySet_.names().size());
        for (const auto &name : querySet_.names())
        {
            // Find the table node for the query. Loop through all the sub-queries until you find
            // one.
            Query foundQuery;
            std::shared_ptr<BufrNode> tableNode;
            for (const auto &query : querySet_.queriesFor(name))
            {
                if (query.subset->isAnySubset ||
                    (query.subset->name == dataProvider_->getSubsetVariant().subset &&
                     query.subset->index == dataProvider_->getSubsetVariant().variantId))
                {
                    tableNode = table.getNodeForPath(query.path);
                    foundQuery = query;
                    if (tableNode != nullptr) break;
                }
            }

            auto target = std::make_shared<Target>();

            // There was no corresponding table node for any of the sub-queries so create empty
            // target.
            if (tableNode == nullptr)
            {
                // Create empty target
                target->name = name;
                target->nodeIdx = 0;
                target->queryStr = querySet_.queriesFor(name)[0].str();
                target->dimPaths.push_back({Query()});
                target->typeInfo = TypeInfo();
                target->exportDimIdxs = {0};
                targets->push_back(target);

                // Print message to inform the user of the missing targets
                log::warning() << "Warning: Query String ";
                log::warning() << querySet_.queriesFor(name)[0].str();
                log::warning() << " did not apply to subset ";
                log::warning() << dataProvider_->getSubsetVariant().str();
                log::warning() << std::endl;

                continue;
            }

            // Create the target
            target->name = name;
            target->queryStr = foundQuery.str();

            // Create the target components
            std::vector<TargetComponent> path(foundQuery.path.size() + 1);

            int pathIdx = 0;
            path[pathIdx].queryComponent = foundQuery.subset;
            path[pathIdx].nodeId = table.getRoot()->nodeIdx;
            path[pathIdx].parentNodeId = 0;
            path[pathIdx].parentDimensionNodeId = 0;
            path[pathIdx].setType(Typ::Subset);
            pathIdx++;

            auto nodes = tableNode->getPathNodes();
            for (size_t nodeIdx = 1; nodeIdx < nodes.size(); nodeIdx++)
            {
                path[pathIdx].queryComponent = foundQuery.path[nodeIdx - 1];
                path[pathIdx].nodeId = nodes[nodeIdx]->nodeIdx;
                path[pathIdx].parentNodeId = nodes[nodeIdx]->getParent()->nodeIdx;
                path[pathIdx].parentDimensionNodeId =
                    nodes[nodeIdx]->getDimensionParent()->nodeIdx;
                path[pathIdx].setType(nodes[nodeIdx]->type);
                path[pathIdx].fixedRepeatCount = nodes[nodeIdx]->fixedRepCount;
                pathIdx++;
            }

            target->setPath(path);
            target->typeInfo = tableNode->typeInfo;
            target->nodeIdx = tableNode->nodeIdx;
            target->longStrId = tableNode->mnemonic + "#" + std::to_string(tableNode->mnemonicIdx);

            targets->push_back(target);
        }

        TargetCacheEntry entry;
        entry.targets = targets;
        entry.layout = SubsetLookupTable::buildLayout(*targets);

        targetsCache_.insert({variant, entry});

        return entry;
    }
}  // namespace bufr
