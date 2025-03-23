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

#include "vkWrappers/wrappers/TopLevelAccelerationStructure.hpp"

namespace vkw
{
TopLevelAccelerationStructure::TopLevelAccelerationStructure(Device& device, const bool buildOnHost)
    : BaseAccelerationStructure()
{
    VKW_CHECK_BOOL_THROW(
        this->init(device, buildOnHost), "Error creating top level acceleration structure");
}

TopLevelAccelerationStructure& TopLevelAccelerationStructure::operator=(
    TopLevelAccelerationStructure&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(storageBuffer_, rhs.storageBuffer_);
    std::swap(type_, rhs.type_);
    std::swap(buildSizes_, rhs.buildSizes_);
    std::swap(accelerationStructure_, rhs.accelerationStructure_);
    std::swap(geometryType_, rhs.geometryType_);
    std::swap(buildOnHost_, rhs.buildOnHost_);

    std::swap(geometry_, rhs.geometry_);
    std::swap(instancesBuffer_, rhs.instancesBuffer_);
    std::swap(instancesList_, rhs.instancesList_);

    return *this;
}

bool TopLevelAccelerationStructure::init(Device& device, const bool buildOnHost)
{
    if(!initialized_)
    {
        device_ = &device;
        buildOnHost_ = buildOnHost;

        initialized_ = true;
    }
    return true;
}

void TopLevelAccelerationStructure::create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags)
{
    // Build geometry list
    VkAccelerationStructureGeometryDataKHR geometryData = {};
    geometryData.instances.sType
        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometryData.instances.pNext = nullptr;
    geometryData.instances.arrayOfPointers = VK_TRUE;
    if(buildOnHost_)
    {
        geometryData.instances.data.hostAddress = reinterpret_cast<void*>(instancesList_.data());
    }
    else
    {
        instancesBuffer_.init(*device_, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, instancesList_.size());
        instancesBuffer_.mapMemory();
        memcpy(
            instancesBuffer_.data(),
            instancesList_.data(),
            instancesList_.size() * sizeof(VkAccelerationStructureInstanceKHR));
        instancesBuffer_.unmapMemory();

        geometryData.instances.data.deviceAddress = instancesBuffer_.deviceAddress();
    }

    geometry_.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry_.pNext = nullptr;
    geometry_.flags = 0;
    geometry_.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry_.geometry = geometryData;

    const uint32_t primitiveCount = static_cast<uint32_t>(instancesList_.size());

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = type_;
    buildInfo.flags = buildFlags;
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry_;
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData = {};
    device_->vk().vkGetAccelerationStructureBuildSizesKHR(
        device_->getHandle(),
        buildOnHost_ ? VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR
                     : VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildInfo,
        &primitiveCount,
        &buildSizes_);

    VKW_CHECK_BOOL_THROW(
        storageBuffer_.init(
            *device_,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
                | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            buildSizes_.accelerationStructureSize),
        "Error initializing TLAS storage buffer");

    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.createFlags = 0; // Other flags are not supported for now
    createInfo.buffer = storageBuffer_.getHandle();
    createInfo.size = buildSizes_.accelerationStructureSize;
    createInfo.type = type();
    createInfo.deviceAddress = 0;
    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateAccelerationStructureKHR(
            device_->getHandle(), &createInfo, nullptr, &accelerationStructure_),
        "Error creating TLAS");
}

void TopLevelAccelerationStructure::clear()
{
    initialized_ = false;

    geometry_ = {};
    instancesBuffer_.clear();
    instancesList_.clear();

    BaseAccelerationStructure::clear();
}

TopLevelAccelerationStructure& TopLevelAccelerationStructure::addInstance(
    const BottomLevelAccelerationStructure& geometry, const VkTransformMatrixKHR& transform)
{
    VKW_CHECK_BOOL_THROW(
        this->buildOnHost() == geometry.buildOnHost(),
        "Error all structures must be build at the same place: device or host");

    VkAccelerationStructureInstanceKHR geometryInstance = {};
    geometryInstance.transform = transform;
    geometryInstance.instanceCustomIndex = 0;                    ///@todo: Add actual value here
    geometryInstance.mask = 0xff;                                ///@todo: Add actual value here
    geometryInstance.instanceShaderBindingTableRecordOffset = 0; ///@todo: Add actual value here
    geometryInstance.accelerationStructureReference
        = geometry.buildOnHost() ? reinterpret_cast<uint64_t>(geometry.getHandle())
                                 : static_cast<uint64_t>(geometry.getDeviceAddress());
    instancesList_.push_back(geometryInstance);

    return *this;
}

void TopLevelAccelerationStructure::build(
    void* scratchData,
    const VkBuildAccelerationStructureFlagsKHR buildFlags,
    const bool /*deferred*/)
{
    VKW_CHECK_BOOL_THROW((buildOnHost_ == true), "Error TLAS not mean to be built on host");

    VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
    buildRange.primitiveCount = static_cast<uint32_t>(instancesList_.size());

    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRanges = &buildRange;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.flags = buildFlags;
    buildInfo.type = type();
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry_;
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData.hostAddress = scratchData;
    VKW_CHECK_VK_THROW(
        device_->vk().vkBuildAccelerationStructuresKHR(
            device_->getHandle(), VK_NULL_HANDLE, 1, &buildInfo, &pBuildRanges),
        "Error buolding TLAS on host");
}
} // namespace vkw