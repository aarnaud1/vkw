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

#include "vkw/detail/AccelerationStructureBuildInfo.hpp"
#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"

namespace vkw
{
class BaseAccelerationStructure
{
  public:
    BaseAccelerationStructure(const BaseAccelerationStructure&) = delete;
    BaseAccelerationStructure(BaseAccelerationStructure&&) = delete;

    BaseAccelerationStructure& operator=(const BaseAccelerationStructure&) = delete;
    BaseAccelerationStructure& operator=(BaseAccelerationStructure&&) = delete;

    virtual ~BaseAccelerationStructure() {}

    auto getHandle() const { return accelerationStructure_; }

    bool buildOnHost() const { return buildOnHost_; }

    VkDeviceAddress getDeviceAddress() const
    {
        VKW_ASSERT(accelerationStructure_ != VK_NULL_HANDLE);

        const VkAccelerationStructureDeviceAddressInfoKHR info
            = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, nullptr,
               accelerationStructure_};
        return device_->vk().vkGetAccelerationStructureDeviceAddressKHR(device_->getHandle(), &info);
    }

    auto& storageBuffer() { return storageBuffer_; }
    const auto& storageBuffer() const { return storageBuffer_; }

    auto storageBufferDeviceAddress() const { return storageBuffer_.deviceAddress(); }

    auto accelerationStructureSize() const { return buildSizes_.accelerationStructureSize; }
    auto updateScratchSize() const { return buildSizes_.updateScratchSize; }
    auto buildScratchSize() const { return buildSizes_.buildScratchSize; }

    virtual VkAccelerationStructureTypeKHR type() const = 0;

  protected:
    const Device* device_{nullptr};

    HostDeviceBuffer<uint8_t> storageBuffer_{};

    VkAccelerationStructureBuildSizesInfoKHR buildSizes_{};
    VkAccelerationStructureKHR accelerationStructure_{VK_NULL_HANDLE};

    GeometryType geometryType_{GeometryType::Undefined};

    bool buildOnHost_{false};

    BaseAccelerationStructure() = default;

    virtual void clear()
    {
        buildOnHost_ = false;
        geometryType_ = GeometryType::Undefined;
        VKW_DELETE_VK(AccelerationStructureKHR, accelerationStructure_);
        buildSizes_ = {};
        storageBuffer_.clear();
        device_ = nullptr;
    }
};
} // namespace vkw