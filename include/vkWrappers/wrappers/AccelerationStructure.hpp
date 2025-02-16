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

#include "vkWrappers/wrappers/AccelerationStructureBuildInfo.hpp"
#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/Device.hpp"

namespace vkw
{
class AccelerationStructure
{
  public:
    bool init(Device& device, const VkAccelerationStructureTypeKHR type, const bool buildOnHost);

    bool isInitialized() const { return initialized_; }

    void create();

    auto getHandle() const { return accelerationStructure_; }

    template <VkFormat format, VkIndexType indexType>
    AccelerationStructure& addGeometryData(
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

    AccelerationStructure& addGeometryData(
        const VkAccelerationStructureGeometryTrianglesDataKHR& data,
        const uint32_t primitiveCount,
        const VkGeometryFlagsKHR flags = 0)
    {
        VkAccelerationStructureGeometryDataKHR geometryData = {};
        geometryData.triangles = data;

        VkAccelerationStructureGeometryKHR geometryInfo = {};
        geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometryInfo.pNext = nullptr;
        geometryInfo.flags = flags;
        geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        geometryInfo.geometry = geometryData;

        geometryData_.emplace_back(geometryInfo);
        primitiveCounts_.emplace_back(primitiveCount);

        return *this;
    }

    AccelerationStructure& addGeometryData(
        const VkAccelerationStructureGeometryAabbsDataKHR& data,
        const uint32_t primitiveCount,
        const VkGeometryFlagsKHR flags = 0)
    {
        VkAccelerationStructureGeometryDataKHR geometryData = {};
        geometryData.aabbs = data;

        VkAccelerationStructureGeometryKHR geometryInfo = {};
        geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometryInfo.pNext = nullptr;
        geometryInfo.flags = flags;
        geometryInfo.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
        geometryInfo.geometry = geometryData;

        geometryData_.emplace_back(geometryInfo);
        primitiveCounts_.emplace_back(primitiveCount);

        return *this;
    }

    AccelerationStructure& addGeometryData(
        const VkAccelerationStructureGeometryInstancesDataKHR& data,
        const uint32_t primitiveCount,
        const VkGeometryFlagsKHR flags = 0)
    {
        VkAccelerationStructureGeometryDataKHR geometryData = {};
        geometryData.instances = data;

        VkAccelerationStructureGeometryKHR geometryInfo = {};
        geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        geometryInfo.pNext = nullptr;
        geometryInfo.flags = flags;
        geometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        geometryInfo.geometry = geometryData;

        geometryData_.emplace_back(geometryInfo);
        primitiveCounts_.emplace_back(primitiveCount);

        return *this;
    }

    auto& storageBuffer() { return storageBuffer_; }
    const auto& storageBuffer() const { return storageBuffer_; }

    void build();
    template <typename ScratchBufferType>
    void build(const ScratchBufferType& scratchBuffer);

    void update();
    template <typename ScratchBufferType>
    void update(const ScratchBufferType& scratchBuffer);

    void copy();

  private:
    Device* device_{nullptr};

    HostDeviceBuffer<uint8_t> storageBuffer_{};

    VkAccelerationStructureTypeKHR type_{};
    VkAccelerationStructureBuildSizesInfoKHR buildSizes_{};
    VkAccelerationStructureKHR accelerationStructure_{VK_NULL_HANDLE};

    std::vector<VkAccelerationStructureGeometryKHR> geometryData_{};
    std::vector<uint32_t> primitiveCounts_{};

    bool buildOnHost_{false};
    bool initialized_{false};

    void initializeBuildSizes();
};
} // namespace vkw