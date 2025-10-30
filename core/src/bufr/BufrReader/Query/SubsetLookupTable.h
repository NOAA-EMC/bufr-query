// (C) Copyright 2023 NOAA/NWS/NCEP/EMC

#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <unordered_set>
#include <vector>

#include "bufr/DataProvider.h"
#include "bufr/Data.h"
#include "Target.h"


namespace bufr {

    namespace __details
    {
        struct SubsetLayout
        {
            size_t minNodeId = 0;
            size_t maxNodeId = 0;
            size_t size = 0;
            std::vector<int32_t> offsets;

            size_t index(size_t nodeId) const;
            bool contains(size_t nodeId) const;
        };

        template <typename T>
        class NodeArray
        {
         public:
            NodeArray() = default;

            explicit NodeArray(const std::shared_ptr<const SubsetLayout>& layout)
                : layout_(layout)
            {
                if (layout_)
                {
                    data_.resize(layout_->size);
                }
            }

            void reset(const std::shared_ptr<const SubsetLayout>& layout)
            {
                layout_ = layout;
                data_.clear();
                if (layout_)
                {
                    data_.resize(layout_->size);
                }
            }

            T& operator[](size_t nodeId)
            {
                return data_.at(layout_->index(nodeId));
            }

            const T& operator[](size_t nodeId) const
            {
                return data_.at(layout_->index(nodeId));
            }

            bool contains(size_t nodeId) const
            {
                return layout_ && layout_->contains(nodeId);
            }

         private:
            std::shared_ptr<const SubsetLayout> layout_;
            std::vector<T> data_;
        };
    }  // namespace __details

    /// \brief Lookup table that maps BUFR subset node ids to the data and counts found in the BUFR
    /// message subset data section. This makes it possible to quickly access the data and counts
    /// information for a given node.
    class SubsetLookupTable
    {
     public:
        typedef std::vector<int> CountsVector;

        struct NodeMetaData
        {
            TargetComponent component;
            std::string longStrId;
            bool collectedCounts = false;
            bool collectedData = false;
        };

        struct NodeData
        {
            Data data;
            CountsVector counts;
        };

        typedef __details::NodeArray<NodeData> LookupTable;
        typedef __details::NodeArray<NodeMetaData> LookupMetaTable;
        using Layout = __details::SubsetLayout;

        SubsetLookupTable(const std::shared_ptr<DataProvider>& dataProvider,
                          const std::shared_ptr<Targets>& targets,
                          const std::shared_ptr<const Layout>& layout);

        static std::shared_ptr<const Layout> buildLayout(const Targets& targets);

        /// \brief Returns the NodeData for a given bufr node.
        /// \param[in] nodeId The id of the node to get the data for.
        /// \return The NodeData for the given node.
        const NodeData& operator[](size_t nodeId) const { return lookupTable_[nodeId]; }

        /// \brief Gets the idx for the target with the given name.
        /// \param[in] name The name of the target to get the idx for.
        /// \return The idx of the target with the given name.
        size_t getTargetIdx(std::string name) const
        {
            size_t idx = 0;
            for (const auto& target : *targets_)
            {
                if (target->name == name) { break; }
                ++idx;
            }

            return idx;
        }

        /// \brief Gets the target at the given idx.
        /// \param[in] idx The idx of the target to get.
        /// \return The target at the given idx.
        std::shared_ptr<Target>& targetAtIdx(size_t idx) const { return targets_->at(idx); }

     private:
        const std::shared_ptr<Targets> targets_;
        std::shared_ptr<const Layout> layout_;
        LookupTable lookupTable_;

        /// \brief Creates a lookup table that maps node ids to NodeData objects.
        /// \param[in] targets The targets to create the lookup table for.
        /// \return The lookup table.
        LookupTable makeLookupTable(const std::shared_ptr<DataProvider>& dataProvider,
                                    const Targets& targets) const;

        /// \brief Adds the counts data for the given targets to the lookup table.
        /// \param[in] targets The targets to add the counts data for.
        /// \param[in, out] lookup The lookup table to add the counts data to.
        void addCounts(const std::shared_ptr<DataProvider>& dataProvider,
                       const Targets& targets,
                       LookupTable& lookup,
                       LookupMetaTable& lookupMeta) const;

        /// \brief Adds the data for the given targets to the lookup table.
        /// \param[in] targets The targets to add the data for.
        /// \param[in, out] lookup The lookup table to add the data to.
        void addData(const std::shared_ptr<DataProvider>& dataProvider,
                     const Targets& targets,
                     LookupTable& lookup,
                     LookupMetaTable& lookupMeta) const;
    };
}  // namespace bufr
