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

#include "vkw/detail/ASGeometryData.hpp"

namespace vkw
{
AccelerationStructureTriangleData::AccelerationStructureTriangleData(
    const VkFormat format, const void* vertexPtr, const void* transformPtr, const uint32_t vertexCount,
    const uint32_t vertexStride)
    : vertexFormat_{format}
    , indexType_{VK_INDEX_TYPE_NONE_KHR}
    , vertexCount_{vertexCount}
    , vertexStride_{vertexStride}
    , primitiveCount_{vertexCount / 3}
    , useHostPtr_{true}
    , useIndices_{false}
{
    vertexBufferAddress_.hostAddress = reinterpret_cast<const void*>(vertexPtr);
    indexBufferAddress_.hostAddress = nullptr;
    transformBufferAddress_.hostAddress = reinterpret_cast<const void*>(transformPtr);
}

AccelerationStructureTriangleData::AccelerationStructureTriangleData(
    const VkFormat format, const void* vertexPtr, const void* indexPtr, const void* transformPtr,
    const uint32_t vertexCount, const VkIndexType indexType, const uint32_t vertexStride,
    const uint32_t primitiveCount)
    : vertexFormat_{format}
    , indexType_{indexType}
    , vertexCount_{vertexCount}
    , vertexStride_{vertexStride}
    , primitiveCount_{primitiveCount}
    , useHostPtr_{true}
    , useIndices_{true}
{
    vertexBufferAddress_.hostAddress = vertexPtr;
    indexBufferAddress_.hostAddress = indexPtr;
    transformBufferAddress_.hostAddress = transformPtr;
}

AccelerationStructureTriangleData::AccelerationStructureTriangleData(
    const VkFormat format, const BaseBuffer& vertexBuffer, const BaseBuffer& transformBuffer,
    const uint32_t vertexCount, const uint32_t vertexStride)
    : vertexFormat_{format}
    , indexType_{VK_INDEX_TYPE_NONE_KHR}
    , vertexCount_{vertexCount}
    , vertexStride_{vertexStride}
    , primitiveCount_{vertexCount / 3}
    , useHostPtr_{false}
    , useIndices_{false}
{
    VKW_ASSERT(
        (vertexBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) > 0);
    VKW_ASSERT(
        (transformBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) > 0);

    vertexBufferAddress_.deviceAddress = vertexBuffer.deviceAddress();
    indexBufferAddress_.deviceAddress = {};
    transformBufferAddress_.deviceAddress = transformBuffer.deviceAddress();
}
AccelerationStructureTriangleData::AccelerationStructureTriangleData(
    const VkFormat format, const BaseBuffer& vertexBuffer, const BaseBuffer& indexBuffer,
    const BaseBuffer& transformBuffer, const uint32_t vertexCount, const VkIndexType indexType,
    const uint32_t vertexStride, const uint32_t primitiveCount)
    : vertexFormat_{format}
    , indexType_{indexType}
    , vertexCount_{vertexCount}
    , vertexStride_{vertexStride}
    , primitiveCount_{primitiveCount}
    , useHostPtr_{false}
    , useIndices_{true}
{
    VKW_ASSERT(
        (vertexBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) > 0);
    VKW_ASSERT(
        (indexBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) > 0);
    VKW_ASSERT(
        (transformBuffer.usage() & VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) > 0);

    vertexBufferAddress_.deviceAddress = vertexBuffer.deviceAddress();
    indexBufferAddress_.deviceAddress = indexBuffer.deviceAddress();
    transformBufferAddress_.deviceAddress = transformBuffer.deviceAddress();
}

VkAccelerationStructureGeometryDataKHR AccelerationStructureTriangleData::geometryData() const
{
    VkAccelerationStructureGeometryTrianglesDataKHR triangleData = {};
    triangleData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangleData.pNext = nullptr;
    triangleData.vertexFormat = vertexFormat_;
    triangleData.vertexData = vertexBufferAddress_;
    triangleData.vertexStride = vertexStride_;
    triangleData.maxVertex = static_cast<uint32_t>(vertexCount_);
    triangleData.indexType = indexType_;
    triangleData.indexData = indexBufferAddress_;
    triangleData.transformData = transformBufferAddress_;

    VkAccelerationStructureGeometryDataKHR ret = {};
    ret.triangles = triangleData;

    return ret;
}
} // namespace vkw