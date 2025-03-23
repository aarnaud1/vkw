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

namespace vkw
{
enum class GeometryType
{
    Instances,
    Triangles,
    Boxes,
    Undefined
};

static constexpr VkTransformMatrixKHR asIdentityMatrix
    = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

// -------------------------------------------------------------------------------------------------

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
        const uint32_t vertexCount,
        const uint32_t vertexStride,
        const uint32_t primitiveCount)
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
        const uint32_t vertexCount,
        const uint32_t vertexStride,
        const uint32_t primitiveCount)
        : useHostPtr_{false}
        , vertexCount_{vertexCount}
        , vertexStride_{vertexStride}
        , primitiveCount_{primitiveCount}
    {
        VKW_CHECK_BOOL_THROW(
            (vertexBuffer.getUsage()
             & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
                > 0,
            "Wrong buffer usage for acceleration structure geometry");
        VKW_CHECK_BOOL_THROW(
            (indexBuffer.getUsage()
             & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
                > 0,
            "Wrong buffer usage for acceleration structure geometry");
        VKW_CHECK_BOOL_THROW(
            (transformBuffer.getUsage()
             & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
                > 0,
            "Wrong buffer usage for acceleration structure geometry");

        vertexBufferAddress_.deviceAddress = vertexBuffer.deviceAddress();
        indexBufferAddress_.deviceAddress = indexBuffer.deviceAddress();
        transformBufferAddress_.deviceAddress = transformBuffer.deviceAddress();
    }

    AccelerationStructureTriangleData(const AccelerationStructureTriangleData&) = default;
    AccelerationStructureTriangleData(AccelerationStructureTriangleData&& rhs) = default;
    AccelerationStructureTriangleData& operator=(const AccelerationStructureTriangleData&)
        = default;
    AccelerationStructureTriangleData& operator=(AccelerationStructureTriangleData&& rhs) = default;

    ~AccelerationStructureTriangleData() = default;

    auto useHostPtr() const { return useHostPtr_; }
    auto vertexCount() const { return vertexCount_; }
    auto vertexStride() const { return vertexStride_; }
    auto primitiveCount() const { return primitiveCount_; }

    static constexpr VkGeometryTypeKHR geometryType() { return VK_GEOMETRY_TYPE_TRIANGLES_KHR; }

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
    uint32_t vertexCount_{0};
    uint32_t vertexStride_{0};
    uint32_t primitiveCount_{0};
    bool useHostPtr_{false};

    VkDeviceOrHostAddressConstKHR vertexBufferAddress_{};
    VkDeviceOrHostAddressConstKHR indexBufferAddress_{};
    VkDeviceOrHostAddressConstKHR transformBufferAddress_{};
};

// -------------------------------------------------------------------------------------------------

// FLOAT16 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec2f16 = AccelerationStructureTriangleData<VK_FORMAT_R16G16_SFLOAT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec3f16
    = AccelerationStructureTriangleData<VK_FORMAT_R16G16B16_SFLOAT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec4f16
    = AccelerationStructureTriangleData<VK_FORMAT_R16G16B16A16_SFLOAT, indexType>;

// UINT16 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec2u16 = AccelerationStructureTriangleData<VK_FORMAT_R16G16_UINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec3u16 = AccelerationStructureTriangleData<VK_FORMAT_R16G16B16_UINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec4u16
    = AccelerationStructureTriangleData<VK_FORMAT_R16G16B16A16_UINT, indexType>;

// INT16 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec2i16 = AccelerationStructureTriangleData<VK_FORMAT_R16G16_SINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec3i16 = AccelerationStructureTriangleData<VK_FORMAT_R16G16B16_SINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT16>
using TriangleDataVec4i16
    = AccelerationStructureTriangleData<VK_FORMAT_R16G16B16A16_SINT, indexType>;

// FLOAT32 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec2f32 = AccelerationStructureTriangleData<VK_FORMAT_R32G32_SFLOAT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec3f32
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_SFLOAT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec4f32
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32A32_SFLOAT, indexType>;

// UINT32 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec2u32 = AccelerationStructureTriangleData<VK_FORMAT_R32G32_UINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec3u32 = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_UINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec4u32
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32A32_UINT, indexType>;

// INT32 vector types
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec2i32 = AccelerationStructureTriangleData<VK_FORMAT_R32G32_SINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec3i32 = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_SINT, indexType>;
template <VkIndexType indexType = VK_INDEX_TYPE_UINT32>
using TriangleDataVec4i32
    = AccelerationStructureTriangleData<VK_FORMAT_R32G32B32A32_SINT, indexType>;
} // namespace vkw