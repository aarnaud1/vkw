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

#include "vkWrappers/wrappers/BaseAccelerationStructure.hpp"

namespace vkw
{
void BaseAccelerationStructure::initializeBuildSizes(
    const VkBuildAccelerationStructureFlagBitsKHR flags)
{
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = type_;
    buildInfo.flags = flags;
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.geometryCount = static_cast<uint32_t>(geometryData_.size());
    buildInfo.pGeometries = geometryData_.data();
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData = {};

    // Build sizes
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    device_->vk().vkGetAccelerationStructureBuildSizesKHR(
        device_->getHandle(),
        buildOnHost_ ? VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR
                     : VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        primitiveCounts_.data(),
        &buildSizes_);

    // Update sizes
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    device_->vk().vkGetAccelerationStructureBuildSizesKHR(
        device_->getHandle(),
        buildOnHost_ ? VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR
                     : VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        primitiveCounts_.data(),
        &updateSizes_);
}

} // namespace vkw