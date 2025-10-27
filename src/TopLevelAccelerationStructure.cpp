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

#include "vkw/detail/TopLevelAccelerationStructure.hpp"

namespace vkw
{
TopLevelAccelerationStructure::TopLevelAccelerationStructure(const Device& device, const bool buildOnHost)
    : BaseAccelerationStructure()
{
    VKW_CHECK_BOOL_FAIL(this->init(device, buildOnHost), "Error creating top level acceleration structure");
}

TopLevelAccelerationStructure& TopLevelAccelerationStructure::operator=(TopLevelAccelerationStructure&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(storageBuffer_, rhs.storageBuffer_);
    std::swap(buildSizes_, rhs.buildSizes_);
    std::swap(accelerationStructure_, rhs.accelerationStructure_);
    std::swap(geometryType_, rhs.geometryType_);
    std::swap(buildOnHost_, rhs.buildOnHost_);

    std::swap(geometry_, rhs.geometry_);
    std::swap(instancesBuffer_, rhs.instancesBuffer_);
    std::swap(instancesList_, rhs.instancesList_);

    return *this;
}

bool TopLevelAccelerationStructure::init(const Device& device, const bool buildOnHost)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    buildOnHost_ = buildOnHost;

    initialized_ = true;

    return true;
}

void TopLevelAccelerationStructure::create(const VkBuildAccelerationStructureFlagBitsKHR buildFlags)
{
    // Build geometry list
    VkAccelerationStructureGeometryDataKHR geometryData = {};
    geometryData.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometryData.instances.pNext = nullptr;
    geometryData.instances.arrayOfPointers = VK_FALSE;
    if(buildOnHost_)
    {
        geometryData.instances.data.hostAddress = reinterpret_cast<void*>(instancesList_.data());
    }
    else
    {
        instancesBuffer_.init(
            *device_, instancesList_.size(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);
        instancesBuffer_.copyFromHost(instancesList_.data(), instancesList_.size());

        geometryData.instances.data.deviceAddress = instancesBuffer_.deviceAddress();
    }

    geometry_.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry_.pNext = nullptr;
    geometry_.flags = 0;
    geometry_.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry_.geometry = geometryData;

    const uint32_t primitiveCount = static_cast<uint32_t>(instancesList_.size());

    buildSizes_.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    buildSizes_.pNext = nullptr;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.type = type();
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
        &buildInfo, &primitiveCount, &buildSizes_);

    VKW_CHECK_BOOL_FAIL(
        storageBuffer_.init(
            *device_, buildSizes_.accelerationStructureSize,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
                | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT),
        "Error initializing TLAS storage buffer");

    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.createFlags = 0; // Other flags are not supported for now
    createInfo.buffer = storageBuffer_.getHandle();
    createInfo.size = buildSizes_.accelerationStructureSize;
    createInfo.type = type();
    createInfo.deviceAddress = 0;
    VKW_CHECK_VK_FAIL(
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
    const BottomLevelAccelerationStructure& geometry, const uint32_t instanceIndex,
    const VkTransformMatrixKHR& transform, const VkGeometryInstanceFlagsKHR flags, const uint32_t mask,
    const uint32_t hitBindingIndex)
{
    VKW_CHECK_BOOL_FAIL(
        this->buildOnHost() == geometry.buildOnHost(),
        "Error all structures must be build at the same place: device or host");

    VkAccelerationStructureInstanceKHR geometryInstance = {};
    geometryInstance.transform = transform;
    geometryInstance.instanceCustomIndex = instanceIndex;
    geometryInstance.mask = mask;
    geometryInstance.instanceShaderBindingTableRecordOffset = hitBindingIndex;
    geometryInstance.flags = flags;
    geometryInstance.accelerationStructureReference
        = geometry.buildOnHost() ? reinterpret_cast<uint64_t>(geometry.getHandle())
                                 : static_cast<uint64_t>(geometry.getDeviceAddress());
    instancesList_.push_back(geometryInstance);

    return *this;
}

bool TopLevelAccelerationStructure::build(
    void* scratchData, const VkBuildAccelerationStructureFlagsKHR buildFlags, const bool /*deferred*/)
{
    VKW_ASSERT(this->buildOnHost_);

    VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
    buildRange.primitiveCount = static_cast<uint32_t>(instancesList_.size());

    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRanges = &buildRange;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.flags = buildFlags;
    buildInfo.type = type();
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure = accelerationStructure_;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry_;
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData.hostAddress = scratchData;
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkBuildAccelerationStructuresKHR(
        device_->getHandle(), VK_NULL_HANDLE, 1, &buildInfo, &pBuildRanges));
    return true;
}

bool TopLevelAccelerationStructure::update(
    void* scratchData, const VkBuildAccelerationStructureFlagsKHR buildFlags, const bool /*deferred*/)
{
    VKW_ASSERT(this->buildOnHost_);

    VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
    buildRange.primitiveCount = static_cast<uint32_t>(instancesList_.size());

    const VkAccelerationStructureBuildRangeInfoKHR* pBuildRanges = &buildRange;

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.flags = buildFlags;
    buildInfo.type = type();
    buildInfo.srcAccelerationStructure = accelerationStructure_;
    buildInfo.dstAccelerationStructure = accelerationStructure_;
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &geometry_;
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData.hostAddress = scratchData;
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkBuildAccelerationStructuresKHR(
        device_->getHandle(), VK_NULL_HANDLE, 1, &buildInfo, &pBuildRanges));

    return true;
}

bool TopLevelAccelerationStructure::update(
    const std::vector<VkTransformMatrixKHR>& transforms, void* scratchData,
    const VkBuildAccelerationStructureFlagsKHR buildFlags, const bool deferred)
{
    VKW_ASSERT(this->buildOnHost_);
    VKW_ASSERT(transforms.size() >= instancesList_.size());

    for(size_t i = 0; i < instancesList_.size(); ++i)
    {
        instancesList_[i].transform = transforms[i];
    }
    return this->update(scratchData, buildFlags, deferred);
}
} // namespace vkw