/*
 * Copyright (c) 2025 Adrien ARNAUD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/MemoryCommon.hpp"

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
    = {{{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}}};

// -----------------------------------------------------------------------------------------------------------
// ------------------------------------- Forward declarations ------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

template <VkFormat format, VkIndexType indexType>
class AccelerationStructureTriangleData;

///@todo: Add AccelerationStructureSphereData

///@todo: Add AccelerationStructureAabbData

///@todo: Add AccelerationStructureLssData

// -----------------------------------------------------------------------------------------------------------

template <VkFormat format, VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR>
class AccelerationStructureTriangleData final
{
  public:
    AccelerationStructureTriangleData() = default;

    template <typename VertexType, typename TransformType>
    explicit AccelerationStructureTriangleData(
        const VertexType* vertexPtr,
        const TransformType* transformPtr,
        const uint32_t vertexCount,
        const uint32_t vertexStride)
        : vertexCount_{vertexCount}
        , vertexStride_{vertexStride}
        , primitiveCount_{primitiveCount / 3}
        , useHostPtr_{true}
        , useIndices_{false}
    {
        static_assert(
            indexType == VK_INDEX_TYPE_NONE_KHR,
            "When non index buffer is used, indexType should be VK_INDEX_TYPE_NONE_KHR");

        vertexBufferAddress_.hostAddress = reinterpret_cast<const void*>(vertexPtr);
        indexBufferAddress_.hostAddress = nullptr;
        transformBufferAddress_.hostAddress = reinterpret_cast<const void*>(transformPtr);
    }
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
        , useIndices_{true}
    {
        static_assert(
            indexType != VK_INDEX_TYPE_NONE_KHR,
            "When an index buffer is used, indexType must be different than VK_INDEX_TYPE_NONE_KHR");

        vertexBufferAddress_.hostAddress = reinterpret_cast<const void*>(vertexPtr);
        indexBufferAddress_.hostAddress = reinterpret_cast<const void*>(indexPtr);
        transformBufferAddress_.hostAddress = reinterpret_cast<const void*>(transformPtr);
    }

    template <typename VertexBufferType, typename TransformBufferType>
    explicit AccelerationStructureTriangleData(
        const VertexBufferType& vertexBuffer,
        const TransformBufferType& transformBuffer,
        const uint32_t vertexCount,
        const uint32_t vertexStride)
        : vertexCount_{vertexCount}
        , vertexStride_{vertexStride}
        , primitiveCount_{vertexCount / 3}
        , useHostPtr_{false}
        , useIndices_{false}
    {
        static_assert(
            indexType == VK_INDEX_TYPE_NONE_KHR,
            "When non index buffer is used, indexType should be VK_INDEX_TYPE_NONE_KHR");

        VKW_ASSERT(
            (vertexBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            > 0);
        VKW_ASSERT(
            (transformBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            > 0);

        vertexBufferAddress_.deviceAddress = vertexBuffer.deviceAddress();
        indexBufferAddress_.deviceAddress = {};
        transformBufferAddress_.deviceAddress = transformBuffer.deviceAddress();
    }
    template <typename VertexBufferType, typename IndexBufferType, typename TransformBufferType>
    explicit AccelerationStructureTriangleData(
        const VertexBufferType& vertexBuffer,
        const IndexBufferType& indexBuffer,
        const TransformBufferType& transformBuffer,
        const uint32_t vertexCount,
        const uint32_t vertexStride,
        const uint32_t primitiveCount)
        : vertexCount_{vertexCount}
        , vertexStride_{vertexStride}
        , primitiveCount_{primitiveCount}
        , useHostPtr_{false}
        , useIndices_{true}
    {
        static_assert(
            indexType != VK_INDEX_TYPE_NONE_KHR,
            "When an index buffer is used, indexType must be different than VK_INDEX_TYPE_NONE_KHR");

        VKW_ASSERT(
            (vertexBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            > 0);
        VKW_ASSERT(
            (indexBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) > 0);
        VKW_ASSERT(
            (transformBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
            > 0);

        vertexBufferAddress_.deviceAddress = vertexBuffer.deviceAddress();
        indexBufferAddress_.deviceAddress = indexBuffer.deviceAddress();
        transformBufferAddress_.deviceAddress = transformBuffer.deviceAddress();
    }

    AccelerationStructureTriangleData(const AccelerationStructureTriangleData&) = default;
    AccelerationStructureTriangleData(AccelerationStructureTriangleData&& rhs) = default;
    AccelerationStructureTriangleData& operator=(const AccelerationStructureTriangleData&) = default;
    AccelerationStructureTriangleData& operator=(AccelerationStructureTriangleData&& rhs) = default;

    ~AccelerationStructureTriangleData() = default;

    auto useHostPtr() const { return useHostPtr_; }
    auto vertexCount() const { return vertexCount_; }
    auto vertexStride() const { return vertexStride_; }
    auto primitiveCount() const { return primitiveCount_; }
    auto hasIndices() const { return useIndices_; }

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
    bool useIndices_{false};

    VkDeviceOrHostAddressConstKHR vertexBufferAddress_{};
    VkDeviceOrHostAddressConstKHR indexBufferAddress_{};
    VkDeviceOrHostAddressConstKHR transformBufferAddress_{};
};
} // namespace vkw