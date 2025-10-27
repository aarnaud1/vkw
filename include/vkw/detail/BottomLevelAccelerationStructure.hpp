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

#include "vkw/detail/BaseAccelerationStructure.hpp"
#include "vkw/detail/Common.hpp"

namespace vkw
{
class BottomLevelAccelerationStructure final : public BaseAccelerationStructure
{
  public:
    BottomLevelAccelerationStructure() : BaseAccelerationStructure{} {}
    BottomLevelAccelerationStructure(const Device& device, const bool buildOnHost = false);

    BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
    BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& rhs) { *this = std::move(rhs); }

    BottomLevelAccelerationStructure& operator=(const BottomLevelAccelerationStructure&) = delete;
    BottomLevelAccelerationStructure& operator=(BottomLevelAccelerationStructure&& rhs);

    ~BottomLevelAccelerationStructure() { this->clear(); }

    bool initialized() const { return initialized_; }

    bool init(const Device& device, const bool buildOnHost = false);

    ///@todo: Consider adding create() that takes a size as input parameter
    bool create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags = {});

    void clear() override;

    inline VkAccelerationStructureTypeKHR type() const override
    {
        return VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    }

    // ---------------------------------------------------------------------------------------------

    template <VkFormat format, VkIndexType indexType>
    BottomLevelAccelerationStructure& addGeometry(
        const AccelerationStructureTriangleData<format, indexType>& data, const VkGeometryFlagsKHR flags = 0)
    {
        VKW_ASSERT(this->initialized_);
        VKW_ASSERT(data.useHostPtr() == buildOnHost_);

        VkAccelerationStructureGeometryDataKHR geometryData = data.geometryData();
        return addGeometry(geometryData.triangles, data.primitiveCount(), flags);
    }

    template <VkFormat format, VkIndexType indexType>
    BottomLevelAccelerationStructure& addGeometry(
        const AccelerationStructureTriangleData<format, indexType>& data,
        const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& ranges,
        const VkGeometryFlagsKHR flags = 0)
    {
        VKW_ASSERT(this->initialized_);
        VKW_ASSERT(data.useHostPtr() == buildOnHost_);

        VkAccelerationStructureGeometryDataKHR geometryData = data.geometryData();
        return addGeometry(geometryData.triangles, ranges, data.primitiveCount(), flags);
    }

    BottomLevelAccelerationStructure& addGeometry(
        const VkAccelerationStructureGeometryTrianglesDataKHR& data, const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    BottomLevelAccelerationStructure& addGeometry(
        const VkAccelerationStructureGeometryTrianglesDataKHR& data,
        const std::vector<VkAccelerationStructureBuildRangeInfoKHR>& ranges, const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    /// @todo: Add addGeometry() with range parameters
    BottomLevelAccelerationStructure& addGeometry(
        const VkAccelerationStructureGeometryAabbsDataKHR& data, const uint32_t maxPrimitiveCount,
        const VkGeometryFlagsKHR flags = 0);

    // ---------------------------------------------------------------------------------------------

    bool build(
        void* scratchData, const VkBuildAccelerationStructureFlagsKHR buildFlags = {},
        const bool deferred = false);

    ///@todo Not implemented yet
    bool copy();

  private:
    friend class CommandBuffer;

    std::vector<uint32_t> primitiveCounts_{};
    std::vector<VkAccelerationStructureGeometryKHR> geometryData_{};
    std::vector<std::vector<VkAccelerationStructureBuildRangeInfoKHR>> buildRanges_{};

    bool initialized_{false};
};
} // namespace vkw