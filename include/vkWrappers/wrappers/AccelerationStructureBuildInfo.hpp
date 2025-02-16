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

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/MemoryCommon.hpp"

#include <memory>

namespace vkw
{
template <VkFormat format, VkIndexType indexType>
class AccelerationStructureTriangleData final
{
  public:
    AccelerationStructureTriangleData() = default;

    template <typename VertexType, typename IndexType, typename TransformType>
    explicit AccelerationStructureTriangleData(
        const VertexType* vertexPtr,
        const IndexType* indexPtr,
        const TransformType* transformPtr,
        const VkDeviceSize vertexCount,
        const VkDeviceSize vertexStride,
        const VkDeviceSize primitiveCount)
        : vertexCount_{vertexCount}
        , vertexStride_{vertexStride}
        , primitiveCount_{primitiveCount}
        , useHostPtr_{true}
    {
        vertexBufferAddress_.hostAddress = reinterpret_cast<const void*>(vertexPtr);
        indexBufferAddress_.hostAddress = reinterpret_cast<const void*>(indexPtr);
        transformBufferAddress_.hostAddress = reinterpret_cast<const void*>(transformPtr);
    }

    template <typename VertexType, typename IndexType, typename TransformType, MemoryType memType>
    explicit AccelerationStructureTriangleData(
        const Buffer<VertexType, memType>& vertexBuffer,
        const Buffer<IndexType, memType>& indexBuffer,
        const Buffer<TransformType, memType>& transformBuffer,
        const VkDeviceSize vertexCount,
        const VkDeviceSize vertexStride,
        const VkDeviceSize primitiveCount)
        : vertexCount_{vertexCount}
        , vertexStride_{vertexStride}
        , primitiveCount_{primitiveCount}
        , useHostPtr_{false}
    {
        if(!vertexBuffer.getUsage()
           & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
        {
            throw std::runtime_error("Wrong buffer usage for acceleration structure geometry");
        }
        if(!indexBuffer.getUsage()
           & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
        {
            throw std::runtime_error("Wrong buffer usage for acceleration structure geometry");
        }
        if(!transformBuffer.getUsage()
           & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
        {
            throw std::runtime_error("Wrong buffer usage for acceleration structure geometry");
        }
        vertexBufferAddress_.deviceAddress = vertexBuffer.deviceAddress();
        indexBufferAddress_.deviceAddress = indexBuffer.deviceAddress();
        transformBufferAddress_.deviceAddress = transformBuffer.deviceAddress();
    }

    AccelerationStructureTriangleData(const AccelerationStructureTriangleData&) = delete;
    AccelerationStructureTriangleData(AccelerationStructureTriangleData&& rhs) = default;
    AccelerationStructureTriangleData& operator=(const AccelerationStructureTriangleData&) = delete;
    AccelerationStructureTriangleData& operator=(AccelerationStructureTriangleData&& rhs) = default;

    ~AccelerationStructureTriangleData() = default;

    auto useHostPtr() const { return useHostPtr_; }
    auto vertexCount() const { return vertexCount_; }
    auto vertexStride() const { return vertexStride_; }
    auto primitiveCount() const { return primitiveCount_; }

    VkGeometryTypeKHR geometryType() const { return VK_GEOMETRY_TYPE_TRIANGLES_KHR; }

    VkAccelerationStructureGeometryDataKHR geometryData() const
    {
        VkAccelerationStructureGeometryTrianglesDataKHR triangleData = {};
        triangleData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        triangleData.pNext = nullptr;
        triangleData.vertexFormat = format;
        triangleData.vertexData = vertexBufferAddress_;
        triangleData.vertexStride = vertexStride_;
        triangleData.maxVertex = static_cast<uint32_t>(vertexCount_);
        triangleData.indexType = indexType;
        triangleData.indexData = indexBufferAddress_;
        triangleData.transformData = transformBufferAddress_;

        VkAccelerationStructureGeometryDataKHR ret = {};
        ret.triangles = triangleData;

        return ret;
    }

  private:
    VkDeviceSize vertexCount_{0};
    VkDeviceSize vertexStride_{0};
    VkDeviceSize primitiveCount_{0};

    bool useHostPtr_{false};
    VkDeviceOrHostAddressConstKHR vertexBufferAddress_{};
    VkDeviceOrHostAddressConstKHR indexBufferAddress_{};
    VkDeviceOrHostAddressConstKHR transformBufferAddress_{};
};

// FLOAT32 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVecf2Data
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32_SFLOAT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec3fData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_SFLOAT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec4fData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32A32_SFLOAT, indexType>;

// UINT32 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec2uData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32_UINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec3uData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_UINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec4uData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32A32_UINT, indexType>;

// INT32 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec2iData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32_SINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec3iData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_SINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using AccelerationStructureVec4iData
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32A32_SINT, indexType>;
} // namespace vkw