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
class BaseAccelerationStructure
{
  public:
    using BuildRangeList = std::vector<VkAccelerationStructureBuildRangeInfoKHR>;

    BaseAccelerationStructure(const BaseAccelerationStructure&) = delete;
    BaseAccelerationStructure(BaseAccelerationStructure&&) = delete;

    BaseAccelerationStructure& operator=(const BaseAccelerationStructure&) = delete;
    BaseAccelerationStructure& operator=(BaseAccelerationStructure&&) = delete;

    virtual ~BaseAccelerationStructure() {}

    auto getHandle() const { return accelerationStructure_; }

    bool buildOnHost() const { return buildOnHost_; }

    VkDeviceAddress getDeviceAddress() const;

    auto& storageBuffer() { return storageBuffer_; }
    const auto& storageBuffer() const { return storageBuffer_; }

    auto storageBufferDeviceAddress() const { return storageBuffer_.deviceAddress(); }

    auto scratchBufferSize() const { return buildSizes_.buildScratchSize; }

    virtual VkAccelerationStructureTypeKHR type() const = 0;

  protected:
    Device* device_{nullptr};

    HostDeviceBuffer<uint8_t> storageBuffer_{};

    VkAccelerationStructureTypeKHR type_{};
    VkAccelerationStructureBuildSizesInfoKHR buildSizes_{};
    VkAccelerationStructureBuildSizesInfoKHR updateSizes_{};
    VkAccelerationStructureKHR accelerationStructure_{VK_NULL_HANDLE};
    std::vector<uint32_t> primitiveCounts_{};

    GeometryType geometryType_{GeometryType::Undefined};
    std::vector<VkAccelerationStructureGeometryKHR> geometryData_{};
    std::vector<BuildRangeList> buildRanges_{};

    bool buildOnHost_{false};

    BaseAccelerationStructure() {}

    void initializeBuildSizes(const VkBuildAccelerationStructureFlagBitsKHR flags);
};
} // namespace vkw