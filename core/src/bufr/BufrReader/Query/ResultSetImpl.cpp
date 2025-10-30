// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#include "ResultSetImpl.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <stdexcept>

#include "eckit/mpi/Comm.h"
#include "eckit/exception/Exceptions.h"

#include "VectorMath.h"
#include "bufr/DataObject.h"
#include "../../DataObjectBuilder.h"
#include "../../Log.h"

namespace bufr {

ResultSetImpl::TargetSeries::TargetSeries(const TargetPtr& targetPtr) :
    target(targetPtr),
    typeInfo(targetPtr ? targetPtr->typeInfo : TypeInfo()),
    data(targetPtr ? targetPtr->typeInfo.isLongString() : false)
{
    if (!targetPtr)
    {
        dataOffsets.push_back(0);
        return;
    }

    const auto levelCount = targetPtr->path.empty() ? 0 : targetPtr->path.size() - 1;
    counts.resize(levelCount);
    countOffsets.resize(levelCount);
    for (auto& offsets : countOffsets)
    {
        offsets.push_back(0);
    }

    dataOffsets.push_back(0);

    dims.resize(targetPtr->exportDimIdxs.size(), 1);
    filteredDims.resize(targetPtr->exportDimIdxs.size(), 0);
    rawDims.resize(levelCount, 0);
    dimPaths = targetPtr->dimPaths.empty() ? std::vector<Query>{Query()} : targetPtr->dimPaths;
}

void ResultSetImpl::ensureSeries(const std::shared_ptr<Targets>& targets)
{
    if (!series_.empty() || !targets)
    {
        return;
    }

    series_.reserve(targets->size());
    for (size_t idx = 0; idx < targets->size(); ++idx)
    {
        auto series = TargetSeries(targets->at(idx));
        seriesIndex_.emplace(targets->at(idx)->name, idx);
        series_.push_back(std::move(series));
    }
}

size_t ResultSetImpl::indexFor(const std::string& name) const
{
    const auto it = seriesIndex_.find(name);
    if (it == seriesIndex_.end())
    {
        throw std::out_of_range("Unknown target name: " + name);
    }

    return it->second;
}

void ResultSetImpl::appendFrame(const SubsetLookupTable& frame,
                                const std::shared_ptr<Targets>& targets)
{
    ensureSeries(targets);
    if (series_.empty())
    {
        ++frameCount_;
        return;
    }

    for (size_t idx = 0; idx < series_.size(); ++idx)
    {
        auto& series = series_[idx];
        const auto& target = series.target;

        bool missing = (!target || target->path.empty());
        if (!missing)
        {
            for (auto it = target->path.begin(); it != target->path.end() - 1; ++it)
            {
                if (frame[it->nodeId].counts.empty())
                {
                    missing = true;
                    break;
                }
            }
        }

        series.missingFrames.push_back(missing);

        if (missing)
        {
            series.dataOffsets.push_back(series.dataOffsets.back());
            for (size_t level = 0; level < series.countOffsets.size(); ++level)
            {
                series.countOffsets[level].push_back(series.countOffsets[level].back());
            }

            continue;
        }

        size_t pathIdx = 0;
        size_t exportIdxIdx = 0;
        for (auto it = target->path.begin(); it != target->path.end() - 1; ++it, ++pathIdx)
        {
            const auto& counts = frame[it->nodeId].counts;
            auto& levelCounts = series.counts[pathIdx];
            levelCounts.insert(levelCounts.end(), counts.begin(), counts.end());
            series.countOffsets[pathIdx].push_back(series.countOffsets[pathIdx].back() + counts.size());

            const auto maxCount = counts.empty() ? 0 : std::max(1, static_cast<int>(*std::max_element(counts.begin(), counts.end())));
            if (pathIdx < series.rawDims.size())
            {
                series.rawDims[pathIdx] = std::max(series.rawDims[pathIdx], maxCount);
            }

            if (exportIdxIdx < target->exportDimIdxs.size() &&
                target->exportDimIdxs[exportIdxIdx] == static_cast<int>(pathIdx))
            {
                series.dims[exportIdxIdx] = std::max(series.dims[exportIdxIdx], maxCount);

                const auto filterSize = static_cast<int>(it->queryComponent->filter.size());
                if (filterSize > 0)
                {
                    series.filteredDims[exportIdxIdx] = std::max(series.filteredDims[exportIdxIdx], filterSize);
                }

                ++exportIdxIdx;
            }
        }

        const auto& fragment = frame[target->nodeIdx].data;
        if (series.data.isLongStr() != fragment.isLongStr())
        {
            series.data.isLongStr(fragment.isLongStr());
        }

        if (fragment.isLongStr())
        {
            series.data.value.strings.insert(series.data.value.strings.end(),
                                             fragment.value.strings.begin(),
                                             fragment.value.strings.end());
        }
        else
        {
            series.data.value.octets.insert(series.data.value.octets.end(),
                                            fragment.value.octets.begin(),
                                            fragment.value.octets.end());
        }

        series.dataOffsets.push_back(series.dataOffsets.back() + fragment.size());

        series.typeInfo.reference = std::min(series.typeInfo.reference, target->typeInfo.reference);
        series.typeInfo.bits = std::max(series.typeInfo.bits, target->typeInfo.bits);
        if (std::abs(target->typeInfo.scale) > series.typeInfo.scale)
        {
            series.typeInfo.scale = target->typeInfo.scale;
        }
        if (series.typeInfo.unit.empty())
        {
            series.typeInfo.unit = target->typeInfo.unit;
        }
    }

    ++frameCount_;
}

std::shared_ptr<DataObjectBase> ResultSetImpl::get(const std::string& fieldName,
                                                   const std::string& groupByFieldName,
                                                   const std::string& overrideType) const
{
    if (frameCount_ == 0)
    {
        static bool printWarning = true;
        if (printWarning)
        {
            log::warning() << "WARNING: ResultSet is empty. Returning empty DataObjects." << std::endl;
            printWarning = false;
        }

        auto data = details::ResultData();
        data.buffer = {};
        data.dims = {0};
        data.dimPaths = {Query()};

        auto object = DataObjectBuilder::make(fieldName,
                                              groupByFieldName,
                                              TypeInfo(),
                                              overrideType,
                                              data.buffer,
                                              data.dims,
                                              data.dimPaths);

        return object;
    }

    const auto targetMetaData = analyzeTarget(fieldName);
    auto data = assembleData(targetMetaData);

    if (!groupByFieldName.empty())
    {
        applyGroupBy(data, targetMetaData, groupByFieldName);
    }

    auto object = DataObjectBuilder::make(fieldName,
                                          groupByFieldName,
                                          targetMetaData->typeInfo,
                                          overrideType,
                                          data.buffer,
                                          data.dims,
                                          data.dimPaths);

    return object;
}

std::string ResultSetImpl::resolveType(const eckit::mpi::Comm& comm,
                                       const std::string& fieldName) const
{
    TypeInfo typeInfo;
    if (!series_.empty())
    {
        typeInfo = analyzeTarget(fieldName)->typeInfo;
    }

    std::vector<int> bits(comm.size());
    std::vector<int> scale(comm.size());
    std::vector<int> reference(comm.size());

    comm.allGather(typeInfo.bits, bits.begin(), bits.end());
    comm.allGather(typeInfo.scale, scale.begin(), scale.end());
    comm.allGather(typeInfo.reference, reference.begin(), reference.end());

    std::vector<std::string> unit(comm.size());
    {
        size_t charsToSend = typeInfo.unit.size();

        size_t charsToReceive = charsToSend;
        comm.allReduce(charsToReceive, charsToReceive, eckit::mpi::Operation::SUM);

        auto sizeArray = std::vector<int>(comm.size());
        comm.allGather(static_cast<int>(charsToSend), sizeArray.begin(), sizeArray.end());

        std::vector<char> rcvBuffer(charsToReceive, 0);
        auto rcvCounts = std::vector<int>(comm.size());

        std::vector<int> displacement(comm.size(), 0);
        for (size_t i = 1; i < comm.size(); i++)
        {
            displacement[i] = displacement[i - 1] + sizeArray[i - 1];
        }

        comm.allGatherv(typeInfo.unit.begin(), typeInfo.unit.end(), rcvBuffer.begin(),
                        sizeArray.data(), displacement.data());

        for (size_t i = 0; i < comm.size(); i++)
        {
            unit[i] = std::string(rcvBuffer.begin() + displacement[i],
                                  rcvBuffer.begin() + displacement[i] + sizeArray[i]);
        }
    }

    const std::vector<std::string> precedence = {"unknown", "string", "uint32", "uint64",
                                                 "int32", "int64", "float", "double"};

    size_t highestPrecedence = 0;
    for (size_t taskIdx = 0; taskIdx < comm.size(); ++taskIdx)
    {
        TypeInfo taskTypeInfo;
        taskTypeInfo.bits = bits[taskIdx];
        taskTypeInfo.scale = scale[taskIdx];
        taskTypeInfo.reference = reference[taskIdx];
        taskTypeInfo.unit = unit[taskIdx];

        auto taskTypeStr = DataObjectBuilder::typeString(taskTypeInfo);
        auto precIt = std::find(precedence.begin(), precedence.end(), taskTypeStr);
        if (precIt != precedence.end())
        {
            auto precIdx = static_cast<size_t>(std::distance(precedence.begin(), precIt));
            highestPrecedence = std::max(highestPrecedence, precIdx);
        }
        else
        {
            std::ostringstream errMsg;
            errMsg << "Unkonwn type " << taskTypeStr << " encountered in ResultSet::resolveType." << std::endl;
            throw eckit::BadParameter(errMsg.str());
        }
    }

    return precedence[highestPrecedence];
}

details::TargetMetaDataPtr ResultSetImpl::analyzeTarget(const std::string& name) const
{
    auto metaData = std::make_shared<details::TargetMetaData>();
    const auto idx = indexFor(name);
    metaData->targetIdx = idx;

    const auto& series = series_.at(idx);
    metaData->missingFrames = series.missingFrames;
    metaData->dims = series.dims;
    metaData->rawDims = series.rawDims;
    metaData->filteredDims = series.filteredDims;
    metaData->dimPaths = series.dimPaths;
    metaData->typeInfo = series.typeInfo;

    if (metaData->dims.empty())
    {
        metaData->dims = {1};
    }

    if (metaData->rawDims.empty())
    {
        metaData->rawDims = {1};
    }

    if (metaData->dimPaths.empty())
    {
        metaData->dimPaths = {Query()};
    }

    if (metaData->missingFrames.size() < frameCount_)
    {
        metaData->missingFrames.resize(frameCount_, true);
    }

    for (size_t dimIdx = 0; dimIdx < metaData->filteredDims.size(); ++dimIdx)
    {
        if (metaData->filteredDims[dimIdx] == 0)
        {
            metaData->filteredDims[dimIdx] = metaData->dims[dimIdx];
        }
    }

    return metaData;
}

details::ResultData ResultSetImpl::assembleData(const details::TargetMetaDataPtr& metaData) const
{
    const auto& series = series_.at(metaData->targetIdx);

    int rowLength = 1;
    for (size_t dimIdx = 1; dimIdx < metaData->rawDims.size(); ++dimIdx)
    {
        rowLength *= metaData->rawDims[dimIdx];
    }
    rowLength = std::max(rowLength, 1);

    const auto totalRows = frameCount_;

    details::ResultData data;
    data.buffer.isLongStr(series.data.isLongStr());
    data.buffer.resize(totalRows * rowLength);
    data.dims = metaData->dims;
    data.rawDims = metaData->rawDims;
    data.dimPaths = metaData->dimPaths;
    data.dims[0] = static_cast<int>(totalRows);
    data.rawDims[0] = static_cast<int>(totalRows);

    bool needsFiltering = false;

    for (size_t frameIdx = 0; frameIdx < totalRows; ++frameIdx)
    {
        if (frameIdx >= series.missingFrames.size() || series.missingFrames[frameIdx])
        {
            continue;
        }

        copyData(data, series, frameIdx, frameIdx * rowLength);

        if (series.target && series.target->usesFilters)
        {
            needsFiltering = true;
        }
    }

    if (needsFiltering)
    {
        int filteredRowLength = 1;
        for (size_t dimIdx = 1; dimIdx < metaData->filteredDims.size(); ++dimIdx)
        {
            filteredRowLength *= metaData->filteredDims[dimIdx];
        }

        auto filteredData = details::ResultData();
        filteredData.buffer.isLongStr(series.data.isLongStr());
        filteredData.buffer.resize(totalRows * filteredRowLength);

        for (size_t frameIdx = 0; frameIdx < totalRows; ++frameIdx)
        {
            size_t inputOffset = frameIdx * rowLength;
            size_t outputOffset = frameIdx * filteredRowLength;
            size_t maxDepth = series.target->path.size() - 1;

            copyFilteredData(filteredData, data, series.target,
                             inputOffset, outputOffset, 1, maxDepth,
                             series.target->filterDataList, false);
        }

        filteredData.dimPaths = metaData->dimPaths;
        filteredData.dims = metaData->filteredDims;
        filteredData.dims[0] = static_cast<int>(totalRows);

        data = std::move(filteredData);
    }

    return data;
}

void ResultSetImpl::copyData(details::ResultData& data,
                             const TargetSeries& series,
                             size_t frameIdx,
                             size_t outputOffset) const
{
    if (!series.target || series.counts.empty())
    {
        return;
    }

    std::vector<size_t> levelCursors(series.counts.size(), 0);
    std::vector<size_t> levelEnds(series.counts.size(), 0);
    for (size_t level = 0; level < series.counts.size(); ++level)
    {
        levelCursors[level] = series.countOffsets[level][frameIdx];
        levelEnds[level] = series.countOffsets[level][frameIdx + 1];
    }

    size_t dataCursor = series.dataOffsets[frameIdx];
    _copyData(data, series, levelCursors, levelEnds, outputOffset, dataCursor, 0, 1);
}

void ResultSetImpl::_copyData(details::ResultData& data,
                              const TargetSeries& series,
                              std::vector<size_t>& levelCursors,
                              std::vector<size_t>& levelEnds,
                              size_t& outputOffset,
                              size_t& dataCursor,
                              const size_t dimIdx,
                              const size_t countNumber) const
{
    size_t totalDimSize = 1;
    for (size_t i = dimIdx; i < data.rawDims.size(); ++i)
    {
        totalDimSize *= static_cast<size_t>(data.rawDims[i]);
    }

    if (!totalDimSize || dimIdx > data.rawDims.size() - 1)
    {
        return;
    }

    auto& counts = series.counts[dimIdx];
    auto& cursor = levelCursors[dimIdx];
    const auto end = levelEnds[dimIdx];

    for (size_t countIdx = 0; countIdx < countNumber; ++countIdx)
    {
        if (cursor >= end)
        {
            outputOffset += totalDimSize;
            continue;
        }

        const auto count = static_cast<size_t>(counts[cursor++]);
        if (count == 0)
        {
            outputOffset += totalDimSize;
            continue;
        }

        if (dimIdx == series.counts.size() - 1)
        {
            if (series.data.isLongStr())
            {
                auto begin = series.data.value.strings.begin() + static_cast<std::ptrdiff_t>(dataCursor);
                auto endIt = begin + static_cast<std::ptrdiff_t>(count);
                std::copy(begin, endIt,
                          data.buffer.value.strings.begin() + static_cast<std::ptrdiff_t>(outputOffset));
            }
            else
            {
                auto begin = series.data.value.octets.begin() + static_cast<std::ptrdiff_t>(dataCursor);
                auto endIt = begin + static_cast<std::ptrdiff_t>(count);
                std::copy(begin, endIt,
                          data.buffer.value.octets.begin() + static_cast<std::ptrdiff_t>(outputOffset));
            }

            dataCursor += count;
            outputOffset += totalDimSize;
        }
        else
        {
            _copyData(data, series, levelCursors, levelEnds, outputOffset, dataCursor,
                      dimIdx + 1, count);
        }
    }
}

void ResultSetImpl::validateGroupByField(const details::TargetMetaDataPtr& targetMetaData,
                                         const details::TargetMetaDataPtr& groupByMetaData) const
{
    const auto& groupByPath = series_.at(groupByMetaData->targetIdx).target->dimPaths.back();
    const auto& targetPath = series_.at(targetMetaData->targetIdx).target->dimPaths.back();

    auto groupByPathComps = splitPath(groupByPath.str());
    auto targetPathComps = splitPath(targetPath.str());

    for (size_t i = 1; i < std::min(groupByPathComps.size(), targetPathComps.size()); i++)
    {
        if (targetPathComps[i] != groupByPathComps[i])
        {
            std::ostringstream errStr;
            errStr << "The GroupBy and Target Fields do not share a common path.\n";
            errStr << "GroupByField path: " << groupByPath.str() << std::endl;
            errStr << "TargetField path: " << targetPath.str() << std::endl;
            throw eckit::BadParameter(errStr.str());
        }
    }
}

void ResultSetImpl::copyFilteredData(details::ResultData& resData,
                                     const details::ResultData& srcData,
                                     const TargetPtr& target,
                                     size_t& inputOffset,
                                     size_t& outputOffset,
                                     size_t depth,
                                     size_t maxDepth,
                                     const FilterDataList& filterDataList,
                                     bool skipResult) const
{
    if (depth == maxDepth)
    {
        if (!skipResult)
        {
            if (resData.buffer.isLongStr())
            {
                resData.buffer.value.strings[outputOffset] = srcData.buffer.value.strings[inputOffset];
            }
            else
            {
                resData.buffer.value.octets[outputOffset] = srcData.buffer.value.octets[inputOffset];
            }

            outputOffset++;
        }

        inputOffset++;

        return;
    }

    const auto& filterData = filterDataList[depth];

    if (filterData.isEmpty)
    {
        for (size_t count = 1; count <= static_cast<size_t>(srcData.rawDims[depth]); count++)
        {
            copyFilteredData(resData, srcData, target, inputOffset, outputOffset, depth + 1, maxDepth,
                             filterDataList, skipResult);
        }
    }
    else
    {
        size_t filterIdx = 0;
        auto nextFilterCount = filterData.filter[filterIdx];
        for (size_t count = 1; count <= static_cast<size_t>(srcData.rawDims[depth]); count++)
        {
            bool skip = skipResult;
            if (!skip)
            {
                if (nextFilterCount == count)
                {
                    filterIdx++;
                    if (filterIdx < filterData.filter.size())
                    {
                        nextFilterCount = filterData.filter.at(filterIdx);
                    }
                }
                else
                {
                    skip = true;
                }
            }

            copyFilteredData(resData, srcData, target, inputOffset, outputOffset, depth + 1, maxDepth,
                             filterDataList, skip);
        }
    }
}

void ResultSetImpl::applyGroupBy(details::ResultData& resData,
                                 const details::TargetMetaDataPtr& targetMetaData,
                                 const std::string& groupByFieldName) const
{
    const auto groupByMetaData = analyzeTarget(groupByFieldName);
    validateGroupByField(targetMetaData, groupByMetaData);

    if (groupByMetaData->dims.size() > targetMetaData->dims.size())
    {
        auto newData = details::ResultData();
        newData.buffer.isLongStr(resData.buffer.isLongStr());
        newData.dims = {resData.dims[0] * product(groupByMetaData->dims)};
        newData.buffer.resize(resData.dims[0] * product(groupByMetaData->dims));

        const auto numTargetVals = static_cast<size_t>(product(targetMetaData->dims));

        if (numTargetVals == 0)
        {
            newData.dimPaths = {targetMetaData->dimPaths.back()};
            resData = std::move(newData);
            return;
        }

        const auto numReps = static_cast<size_t>(product(groupByMetaData->dims) / numTargetVals);

        for (size_t targIdx = 0; targIdx < numTargetVals * resData.dims[0]; targIdx++)
        {
            for (size_t rep = 0; rep < numReps; rep++)
            {
                if (resData.buffer.isLongStr())
                {
                    newData.buffer.value.strings[targIdx * numReps + rep]
                        = resData.buffer.value.strings[targIdx];
                }
                else
                {
                    newData.buffer.value.octets[targIdx * numReps + rep]
                        = resData.buffer.value.octets[targIdx];
                }
            }
        }

        newData.dimPaths = {targetMetaData->dimPaths.back()};
        resData = std::move(newData);
    }
    else
    {
        const auto sizeDiff = targetMetaData->dims.size() - groupByMetaData->dims.size();
        auto newDims = std::vector<int>(sizeDiff + 1);
        auto newDimPaths = std::vector<Query>(sizeDiff + 1);

        newDims[0] = resData.dims[0] * product(groupByMetaData->dims);
        newDimPaths[0] = targetMetaData->dimPaths.front();
        for (size_t i = 1; i < sizeDiff + 1; i++)
        {
            newDims[i] = targetMetaData->dims[groupByMetaData->dims.size() + i - 1];
            newDimPaths[i] = targetMetaData->dimPaths[groupByMetaData->dims.size() + i - 1];
        }

        resData.dims = std::move(newDims);
        resData.dimPaths = std::move(newDimPaths);
    }
}

std::string ResultSetImpl::unit(const std::string& fieldName) const
{
    return series_.at(indexFor(fieldName)).typeInfo.unit;
}

std::shared_ptr<DataObjectBase> ResultSetImpl::makeDataObject(
    const std::string& fieldName, const std::string& groupByFieldName, const TypeInfo& info,
    const std::string& overrideType, const Data& data, const std::vector<int>& dims,
    const std::vector<Query>& dimPaths) const
{
    std::shared_ptr<DataObjectBase> object;
    if (overrideType.empty())
    {
        object = objectByTypeInfo(info);
    }
    else
    {
        object = objectByType(overrideType);

        if ((overrideType == "string" && !info.isString())
            || (overrideType != "string" && info.isString()))
        {
            std::ostringstream errMsg;
            errMsg << "Conversions between numbers and strings are not currently supported. ";
            errMsg << "See the export definition for \"" << fieldName << "\".";
            throw eckit::BadParameter(errMsg.str());
        }
    }

    object->setData(data);
    object->setDims(dims);
    object->setFieldName(fieldName);
    object->setGroupByFieldName(groupByFieldName);
    object->setDimPaths(dimPaths);

    return object;
}

std::shared_ptr<DataObjectBase> ResultSetImpl::objectByTypeInfo(const TypeInfo& info) const
{
    std::shared_ptr<DataObjectBase> object;

    if (info.isString() || info.isLongString())
    {
        object = std::make_shared<DataObject<std::string>>();
    }
    else if (info.isInteger())
    {
        if (info.isSigned())
        {
            if (info.is64Bit())
            {
                object = std::make_shared<DataObject<int64_t>>();
            }
            else
            {
                object = std::make_shared<DataObject<int32_t>>();
            }
        }
        else
        {
            if (info.is64Bit())
            {
                object = std::make_shared<DataObject<uint64_t>>();
            }
            else
            {
                object = std::make_shared<DataObject<uint32_t>>();
            }
        }
    }
    else
    {
        if (info.is64Bit())
        {
            object = std::make_shared<DataObject<double>>();
        }
        else
        {
            object = std::make_shared<DataObject<float>>();
        }
    }

    return object;
}

std::shared_ptr<DataObjectBase> ResultSetImpl::objectByType(const std::string& overrideType) const
{
    std::shared_ptr<DataObjectBase> object;

    if (overrideType == "int" || overrideType == "int32")
    {
        object = std::make_shared<DataObject<int32_t>>();
    }
    else if (overrideType == "float" || overrideType == "float32")
    {
        object = std::make_shared<DataObject<float>>();
    }
    else if (overrideType == "double" || overrideType == "float64")
    {
        object = std::make_shared<DataObject<double>>();
    }
    else if (overrideType == "string")
    {
        object = std::make_shared<DataObject<std::string>>();
    }
    else if (overrideType == "int64")
    {
        object = std::make_shared<DataObject<int64_t>>();
    }
    else if (overrideType == "uint64")
    {
        object = std::make_shared<DataObject<uint64_t>>();
    }
    else if (overrideType == "uint32" || overrideType == "uint")
    {
        object = std::make_shared<DataObject<uint32_t>>();
    }
    else if (overrideType == "unknown")
    {
        object = std::make_shared<DataObject<int32_t>>();
    }
    else
    {
        std::ostringstream errMsg;
        errMsg << "Unknown or unsupported type " << overrideType << ".";
        throw eckit::BadParameter(errMsg.str());
    }

    return object;
}

std::vector<std::string> ResultSetImpl::splitPath(const std::string& path)
{
    std::vector<std::string> components;
    std::string::size_type start = 0;
    std::string::size_type end = 0;

    while ((end = path.find('/', start)) != std::string::npos)
    {
        if (end != start)
        {
            components.push_back(path.substr(start, end - start));
        }

        start = end + 1;
    }

    if (start < path.size())
    {
        components.push_back(path.substr(start));
    }

    return components;
}

}  // namespace bufr
