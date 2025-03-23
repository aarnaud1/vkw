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
    BottomLevelAccelerationStructure(Device& device, const bool buildOnHost = false);

    BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
    BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& rhs)
    {
        *this = std::move(rhs);
    }

    BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&) = delete;
    BottomLevelAccelerationStructure& operator=(BottomLevelAccelerationStructure&& rhs);

    ~BottomLevelAccelerationStructure() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    bool init(Device& device, const bool buildOnHost = false);

    ///@todo: Consider adding create() that takes a size as input parameter
    void create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags = {});

    void clear() override;

    inline VkAccelerationStructureTypeKHR type() const override
    {
        return VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    }

    // ---------------------------------------------------------------------------------------------

    template <VkFormat format, VkIndexType indexType>
    BottomLevelAccelerationStructure& addGeometry(
        const AccelerationStructureTriangleData<format, indexType>& data,
        const VkGeometryFlagsKHR flags = 0)
    {
        if(data.useHostPtr() != buildOnHost_)
        {
            throw std::runtime_error(
                "Geometry data must have the same build type as its acceleration structure");
        }
        VkAccelerationStructureGeometryDataKHR geometryData = data.geometryData();
        return addGeometry(geometryData.triangles, data.primitiveCount(), flags);
    }

    /// @todo: Add addGeometry() with range parameters
    BottomLevelAccelerationStructure& addGeometry(
        const VkAccelerationStructureGeometryTrianglesDataKHR& data,
        const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    /// @todo: Add addGeometry() with range parameters
    BottomLevelAccelerationStructure& addGeometry(
        const VkAccelerationStructureGeometryAabbsDataKHR& data,
        const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    // ---------------------------------------------------------------------------------------------

    void build(
        void* scratchData,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {},
        const bool deferred = false);

    ///@todo Not implemented yet
    template <typename ScratchBufferType>
    void update(const ScratchBufferType& scratchBuffer);

    ///@todo Not implemented yet
    void update();

    ///@todo Not implemented yet
    void copy();

  private:
    friend class CommandBuffer;

    std::vector<uint32_t> primitiveCounts_{};
    std::vector<VkAccelerationStructureGeometryKHR> geometryData_{};
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> buildRanges_{};

    bool initialized_{false};
};
} // namespace vkw