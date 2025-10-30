// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bufr/DataObject.h"
#include "bufr/Data.h"
#include "bufr/DataProvider.h"
#include "SubsetLookupTable.h"
#include "Target.h"


namespace bufr {

namespace details
{
    struct TargetMetaData
    {
        size_t targetIdx;
        TypeInfo typeInfo;
        std::vector<int> dims = {0};
        std::vector<int> rawDims = {0};
        std::vector<int> filteredDims = {0};
        std::vector<int> groupedDims = {};
        std::vector<char> missingFrames;
        std::vector<Query> dimPaths;
    };

    struct ResultData
    {
        Data buffer;
        std::vector<int> dims;
        std::vector<int> rawDims;
        std::vector<Query> dimPaths;
    };

    typedef std::shared_ptr<TargetMetaData> TargetMetaDataPtr;

}  // namespace details

    typedef SubsetLookupTable Frame;
    typedef std::vector<Frame> Frames;

    /// \brief This class acts as the container for all the data that is collected during the
    /// the BUFR querying process in the form of SubsetLookupTable instances.
    ///
    /// \par The getter functions for the data construct the final output based on the data and
    /// metadata in these lookup tables. There are many complications. For one the data may be
    /// jagged (lookup table instances do not necessarily all have the same number of elements
    /// [repeated data could have a different number of repeats per instance]). Another is the
    /// application group_by fields which affect the dimensionality of the data. In order to make
    /// the data into rectangular arrays it may be necessary to strategically fill in missing values
    /// so that the data is organized correctly in each dimension.
    ///
    class ResultSetImpl {
     public:
       ResultSetImpl() = default;
        ~ResultSetImpl() = default;

        /// \brief Gets the resulting data for a specific field with a given name grouped by the
        /// optional groupByFieldName.
        /// \param fieldName The name of the field to get the data for.
        /// \param groupByFieldName The name of the field to group the data by.
        /// \param overrideType The name of the override type to convert the data to. Possible
        /// values are int, uint, int32, uint32, int64, uint64, float, double
        /// \return A Result object containing the data.
        std::shared_ptr<DataObjectBase>
        get(const std::string& fieldName,
            const std::string& groupByFieldName = "",
            const std::string& overrideType = "") const;

        /// \brief Discover the type of a field in the result set.
        /// \param comm The MPI communicator to use for resolving the type.
        /// \param fieldName The name of the field to resolve the type for.
        /// \return The type of the field as a string.
        std::string  resolveType(const eckit::mpi::Comm& comm,
                                 const std::string& fieldName) const;

        friend class QueryRunner;

     private:
        struct TargetSeries
        {
            TargetPtr target;
            TypeInfo typeInfo;
            Data data;
            std::vector<size_t> dataOffsets;
            std::vector<std::vector<int>> counts;
            std::vector<std::vector<size_t>> countOffsets;
            std::vector<char> missingFrames;
            std::vector<int> dims;
            std::vector<int> rawDims;
            std::vector<int> filteredDims;
            std::vector<Query> dimPaths;

            TargetSeries() = default;
            explicit TargetSeries(const TargetPtr& targetPtr);
        };

        std::vector<TargetSeries> series_;
        std::unordered_map<std::string, size_t> seriesIndex_;
        size_t frameCount_ = 0;

        details::TargetMetaDataPtr analyzeTarget(const std::string& name) const;
        details::ResultData assembleData(const details::TargetMetaDataPtr& targetMetaData) const;

        void copyData(details::ResultData& data,
                      const TargetSeries& series,
                      size_t frameIdx,
                      size_t outputOffset) const;

        void _copyData(details::ResultData& data,
                       const TargetSeries& series,
                       std::vector<size_t>& levelCursors,
                       std::vector<size_t>& levelEnds,
                       size_t& outputOffset,
                       size_t& dataCursor,
                       const size_t dimIdx,
                       const size_t countNumber) const;

        void validateGroupByField(const details::TargetMetaDataPtr& targetMetaData,
                                  const details::TargetMetaDataPtr& groupByMetaData) const;

        void copyFilteredData(details::ResultData& resData,
                              const details::ResultData& srcData,
                              const TargetPtr& target,
                              size_t& inputOffset,
                              size_t& outputOffset,
                              size_t depth,
                              size_t maxDepth,
                              const FilterDataList& filterDataList,
                              bool skipResult) const;

        /// \brief Modify the ResultData object to apply the group_by field.
        /// \param resData The ResultData object to modify.
        /// \param targetMetaData The metadata for the target.
        /// \param groupByFieldName The name of the field to group the data by.
        void applyGroupBy(details::ResultData& resData,
                          const details::TargetMetaDataPtr& targetMetaData,
                          const std::string& groupByFieldName) const;

        /// \brief Is the field a string field?
        /// \param fieldName The name of the field.
        std::string unit(const std::string& fieldName) const;

        /// \brief Make an appropriate DataObject for the data considering all the META data
        /// \param fieldName The name of the field to get the data for.
        /// \param groupByFieldName The name of the field to group the data by.
        /// \param info The meta data for the element.
        /// \param overrideType The name of the override type to convert the data to. Possible
        /// values are int, uint, int32, uint32, int64, uint64, float, double
        /// \param data The data
        /// \param dims The dimensioning information
        /// \param dimPaths The sub-query path strings for each dimension.
        /// \return A Result DataObject containing the data.
        std::shared_ptr<DataObjectBase> makeDataObject(
                                const std::string& fieldName,
                                const std::string& groupByFieldName,
                                const TypeInfo& info,
                                const std::string& overrideType,
                                const Data& data,
                                const std::vector<int>& dims,
                                const std::vector<Query>& dimPaths) const;

        /// \brief Make an appropriate DataObject for data with the TypeInfo
        /// \param info The meta data for the element.
        /// \return A Result DataObject containing the data.
        std::shared_ptr<DataObjectBase> objectByTypeInfo(const TypeInfo& info) const;

        /// \brief Make an appropriate DataObject for data with the override type
        /// \param overrideType The meta data for the element.
        /// \return A Result DataObject containing the data.
        std::shared_ptr<DataObjectBase> objectByType(const std::string& overrideType) const;

        void ensureSeries(const std::shared_ptr<Targets>& targets);
        size_t indexFor(const std::string& name) const;
        void appendFrame(const SubsetLookupTable& frame, const std::shared_ptr<Targets>& targets);

        /// \brief Utility function that can be used to split a query string into its components.
        /// \param query The query string.
        /// \return A std::string vector to store the components in.
        static std::vector<std::string> splitPath(const std::string& query);
    };
}  // namespace bufr
