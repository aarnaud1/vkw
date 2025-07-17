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
#include "vkw/detail/BottomLevelAccelerationStructure.hpp"
#include "vkw/detail/Common.hpp"

namespace vkw
{
class TopLevelAccelerationStructure final : public BaseAccelerationStructure
{
  public:
    TopLevelAccelerationStructure() : BaseAccelerationStructure{} {}
    TopLevelAccelerationStructure(const Device& device, const bool buildOnHost = false);

    TopLevelAccelerationStructure(const TopLevelAccelerationStructure&) = delete;
    TopLevelAccelerationStructure(TopLevelAccelerationStructure&& rhs) { *this = std::move(rhs); }

    TopLevelAccelerationStructure& operator=(const TopLevelAccelerationStructure&) = delete;
    TopLevelAccelerationStructure& operator=(TopLevelAccelerationStructure&& rhs);

    ~TopLevelAccelerationStructure() { this->clear(); }

    bool initialized() const { return initialized_; }

    bool init(const Device& device, const bool buildOnHost = false);

    void create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags = {});

    void clear() override;

    auto& instances() { return instancesList_; }
    const auto& instances() const { return instancesList_; }

    inline VkAccelerationStructureTypeKHR type() const override
    {
        return VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    }

    // ---------------------------------------------------------------------------------------------

    TopLevelAccelerationStructure& addInstance(
        const BottomLevelAccelerationStructure& geometry,
        const uint32_t instanceIndex,
        const VkTransformMatrixKHR& transform,
        const VkGeometryInstanceFlagsKHR flags = {},
        const uint32_t mask = 0,
        const uint32_t hitBindingIndex = 0);

    // ---------------------------------------------------------------------------------------------

    bool build(
        void* scratchData,
        const VkBuildAccelerationStructureFlagsKHR buildFlags,
        const bool deferred = false);

    bool update(
        void* scratchData,
        const VkBuildAccelerationStructureFlagsKHR buildFlags,
        const bool deferred = false);

    bool update(
        const std::vector<VkTransformMatrixKHR>& transforms,
        void* scratchData,
        const VkBuildAccelerationStructureFlagsKHR buildFlags,
        const bool deferred = false);

    ///@todo Not implemented yet
    bool copy();

  private:
    friend class CommandBuffer;

    VkAccelerationStructureGeometryKHR geometry_{};
    HostBuffer<VkAccelerationStructureInstanceKHR> instancesBuffer_{};
    std::vector<VkAccelerationStructureInstanceKHR> instancesList_{};

    bool initialized_{false};
};
} // namespace vkw