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
        const VkAccelerationStructureDeviceAddressInfoKHR info
            = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
               nullptr,
               accelerationStructure_};
        return device_->vk().vkGetAccelerationStructureDeviceAddressKHR(
            device_->getHandle(), &info);
    }

    auto& storageBuffer() { return storageBuffer_; }
    const auto& storageBuffer() const { return storageBuffer_; }

    auto storageBufferDeviceAddress() const { return storageBuffer_.deviceAddress(); }

    auto accelerationStructureSize() const { return buildSizes_.accelerationStructureSize; }
    auto updateScratchSize() const { return buildSizes_.updateScratchSize; }
    auto buildScratchSize() const { return buildSizes_.buildScratchSize; }

    virtual VkAccelerationStructureTypeKHR type() const = 0;

  protected:
    Device* device_{nullptr};

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