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

#include "vkWrappers/wrappers/AccelerationStructure.hpp"

namespace vkw
{
bool AccelerationStructure::init(
    Device& device, const VkAccelerationStructureTypeKHR type, const bool buildOnHost)
{
    if(!initialized_)
    {
        device_ = &device;
        buildOnHost_ = buildOnHost;
        type_ = type;

        initialized_ = true;
    }
    return true;
}

void AccelerationStructure::create()
{
    initializeBuildSizes();

    storageBuffer_.init(
        *device_,
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        buildSizes_.accelerationStructureSize);

    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.createFlags = 0;
    createInfo.buffer = VK_NULL_HANDLE;
}

void AccelerationStructure::initializeBuildSizes()
{
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = type_;
    ///@todo We should support more flags here
    buildInfo.flags = 0;
    ///@todo We should support other modes
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.geometryCount = static_cast<uint32_t>(geometryData_.size());
    buildInfo.pGeometries = geometryData_.data();
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData = {};

    device_->vk().vkGetAccelerationStructureBuildSizesKHR(
        device_->getHandle(),
        buildOnHost_ ? VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR
                     : VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        primitiveCounts_.data(),
        &buildSizes_);
}
} // namespace vkw