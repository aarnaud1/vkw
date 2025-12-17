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

class AccelerationStructureTriangleData final
{
  public:
    AccelerationStructureTriangleData() = delete;

    explicit AccelerationStructureTriangleData(
        const VkFormat format, const void* vertexPtr, const void* transformPtr, const uint32_t vertexCount,
        const uint32_t vertexStride);
    explicit AccelerationStructureTriangleData(
        const VkFormat format, const void* vertexPtr, const void* indexPtr, const void* transformPtr,
        const uint32_t vertexCount, const VkIndexType indexType, const uint32_t vertexStride,
        const uint32_t primitiveCount);

    explicit AccelerationStructureTriangleData(
        const VkFormat format, const BaseBuffer& vertexBuffer, const BaseBuffer& transformBuffer,
        const uint32_t vertexCount, const uint32_t vertexStride);
    explicit AccelerationStructureTriangleData(
        const VkFormat format, const BaseBuffer& vertexBuffer, const BaseBuffer& indexBuffer,
        const BaseBuffer& transformBuffer, const uint32_t vertexCount, const VkIndexType indexType,
        const uint32_t vertexStride, const uint32_t primitiveCount);

    AccelerationStructureTriangleData(const AccelerationStructureTriangleData&) = default;
    AccelerationStructureTriangleData(AccelerationStructureTriangleData&& rhs) = default;
    AccelerationStructureTriangleData& operator=(const AccelerationStructureTriangleData&) = default;
    AccelerationStructureTriangleData& operator=(AccelerationStructureTriangleData&& rhs) = default;

    ~AccelerationStructureTriangleData() = default;

    inline auto useHostPtr() const { return useHostPtr_; }
    inline auto vertexCount() const { return vertexCount_; }
    inline auto vertexStride() const { return vertexStride_; }
    inline auto primitiveCount() const { return primitiveCount_; }
    inline auto hasIndices() const { return useIndices_; }

    static constexpr VkGeometryTypeKHR geometryType() { return VK_GEOMETRY_TYPE_TRIANGLES_KHR; }

    [[nodiscard]] VkAccelerationStructureGeometryDataKHR geometryData() const;

  private:
    const VkFormat vertexFormat_;
    const VkIndexType indexType_;

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