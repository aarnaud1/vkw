/*
 * Copyright (C) 2025 Adrien ARNAUD
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "vkWrappers/wrappers/BaseAccelerationStructure.hpp"
#include "vkWrappers/wrappers/Common.hpp"

namespace vkw
{
class BottomLevelAccelerationStructure final : public BaseAccelerationStructure
{
  public:
    BottomLevelAccelerationStructure() : BaseAccelerationStructure{} {}
    BottomLevelAccelerationStructure(Device& device, const bool buildOnHost);

    BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
    BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& rhs)
    {
        *this = std::move(rhs);
    }

    BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&) = delete;
    BottomLevelAccelerationStructure& operator=(BottomLevelAccelerationStructure&& rhs);

    ~BottomLevelAccelerationStructure() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    inline VkAccelerationStructureTypeKHR type() const override
    {
        return VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    }

    bool init(Device& device, const bool buildOnHost);

    void create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags = {});

    void clear();

    size_t scratchBufferSize() const { return buildSizes_.buildScratchSize; }

    // Add geometry
    template <VkFormat format, VkIndexType indexType>
    BottomLevelAccelerationStructure& addGeometryData(
        const AccelerationStructureTriangleData<format, indexType>& data,
        const VkGeometryFlagsKHR flags = 0)
    {
        if(data.useHostPtr() != buildOnHost_)
        {
            throw std::runtime_error(
                "Geometry data must have the same build type as its acceleration structure");
        }
        VkAccelerationStructureGeometryDataKHR geometryData = data.geometryData();
        return addGeometryData(geometryData.triangles, data.primitiveCount(), flags);
    }

    BottomLevelAccelerationStructure& addGeometryData(
        const VkAccelerationStructureGeometryTrianglesDataKHR& data,
        const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    BottomLevelAccelerationStructure& addGeometryData(
        const VkAccelerationStructureGeometryAabbsDataKHR& data,
        const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    // Add geometry ranges
    BottomLevelAccelerationStructure& addGeometryRange(
        const size_t geometryIndex, const VkAccelerationStructureBuildRangeInfoKHR& range)
    {
        if(geometryIndex >= geometryData_.size())
        {
            throw std::runtime_error("Invalid geometry index");
        }

        buildRanges_[geometryIndex].push_back(range);

        return *this;
    }

    BottomLevelAccelerationStructure& addGeometryRanges(
        const size_t geometryIndex, const BuildRangeList& ranges)
    {
        if((geometryIndex + ranges.size()) >= geometryData_.size())
        {
            throw std::runtime_error("Invalid geometry index");
        }

        auto& rangeList = buildRanges_[geometryIndex];
        rangeList.insert(rangeList.end(), ranges.begin(), ranges.end());

        return *this;
    }

    void build(void* scratchData, const VkBuildAccelerationStructureFlagsKHR buildFlags);

    ///@todo Not implemented yet
    template <typename ScratchBufferType>
    void update(const ScratchBufferType& scratchBuffer);

    ///@todo Not implemented yet
    void update();

    ///@todo Not implemented yet
    void copy();

  private:
    std::vector<const VkAccelerationStructureGeometryKHR*> ppGeometries_;
    std::vector<const VkAccelerationStructureBuildRangeInfoKHR*> ppBuildRanges_;

    bool initialized_{false};
};
} // namespace vkw