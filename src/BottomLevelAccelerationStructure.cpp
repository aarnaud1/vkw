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

#include "vkw/detail/BottomLevelAccelerationStructure.hpp"

namespace vkw
{
BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(
    Device& device, const bool buildOnHost)
    : BaseAccelerationStructure()
{
    VKW_CHECK_BOOL_FAIL(
        this->init(device, buildOnHost), "Error creating bottom level acceleration structure");
}

BottomLevelAccelerationStructure& BottomLevelAccelerationStructure::operator=(
    BottomLevelAccelerationStructure&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(storageBuffer_, rhs.storageBuffer_);
    std::swap(buildSizes_, rhs.buildSizes_);
    std::swap(accelerationStructure_, rhs.accelerationStructure_);
    std::swap(geometryType_, rhs.geometryType_);
    std::swap(buildOnHost_, rhs.buildOnHost_);

    std::swap(primitiveCounts_, rhs.primitiveCounts_);
    std::swap(geometryData_, rhs.geometryData_);
    std::swap(buildRanges_, rhs.buildRanges_);

    return *this;
}

bool BottomLevelAccelerationStructure::init(Device& device, const bool buildOnHost)
{
    if(!initialized_)
    {
        device_ = &device;
        buildOnHost_ = buildOnHost;

        initialized_ = true;
    }
    return true;
}

void BottomLevelAccelerationStructure::create(
    const VkBuildAccelerationStructureFlagBitsKHR buildFlags)
{
    buildSizes_ = {};
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

    VKW_CHECK_BOOL_FAIL(
        storageBuffer_.init(
            *device_,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
                | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            buildSizes_.accelerationStructureSize),
        "Error initializing BLAS storage buffer");

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
        "Error creating BLAS");
}

void BottomLevelAccelerationStructure::clear()
{
    initialized_ = false;
    geometryData_.clear();
    buildRanges_.clear();
    primitiveCounts_.clear();

    BaseAccelerationStructure::clear();
}

BottomLevelAccelerationStructure& BottomLevelAccelerationStructure::addGeometry(
    const VkAccelerationStructureGeometryTrianglesDataKHR& data,
    const uint32_t maxPrimitiveCount,
    const VkGeometryFlagsKHR flags)
{
    if((geometryType_ != GeometryType::Undefined) && (geometryType_ != GeometryType::Triangles))
    {
        throw std::runtime_error(
            "Error geometry data must be the same in the same acceleration structure");
    }
    this->geometryType_ = GeometryType::Triangles;

    VkAccelerationStructureGeometryDataKHR geometryData = {};
    geometryData.triangles = data;

    VkAccelerationStructureGeometryKHR geometryInfo = {};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometryInfo.pNext = nullptr;
    geometryInfo.flags = flags;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometryInfo.geometry = geometryData;

    primitiveCounts_.emplace_back(maxPrimitiveCount);
    geometryData_.emplace_back(geometryInfo);

    VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
    buildRange.firstVertex = 0;
    buildRange.primitiveCount = maxPrimitiveCount;
    buildRange.primitiveOffset = 0;
    buildRange.transformOffset = 0;
    buildRanges_.emplace_back(buildRange);

    return *this;
}

BottomLevelAccelerationStructure& BottomLevelAccelerationStructure::addGeometry(
    const VkAccelerationStructureGeometryAabbsDataKHR& data,
    const uint32_t maxPrimitiveCount,
    const VkGeometryFlagsKHR flags)
{
    if((geometryType_ != GeometryType::Undefined) && (geometryType_ != GeometryType::Boxes))
    {
        throw std::runtime_error(
            "Error geometry data must be the same in the same acceleration structure");
    }
    this->geometryType_ = GeometryType::Boxes;

    VkAccelerationStructureGeometryDataKHR geometryData = {};
    geometryData.aabbs = data;

    VkAccelerationStructureGeometryKHR geometryInfo = {};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometryInfo.pNext = nullptr;
    geometryInfo.flags = flags;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
    geometryInfo.geometry = geometryData;

    primitiveCounts_.emplace_back(maxPrimitiveCount);
    geometryData_.emplace_back(geometryInfo);

    VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
    buildRange.firstVertex = 0;
    buildRange.primitiveCount = maxPrimitiveCount;
    buildRange.primitiveOffset = 0;
    buildRange.transformOffset = 0;
    buildRanges_.emplace_back(buildRange);

    return *this;
}

void BottomLevelAccelerationStructure::build(
    void* scratchData,
    const VkBuildAccelerationStructureFlagsKHR buildFlags,
    const bool /*deferred*/)
{
    VKW_CHECK_BOOL_FAIL((buildOnHost_ == true), "Error BLAS not mean to be built on host");
    VKW_CHECK_BOOL_FAIL(geometryData_.size() == buildRanges_.size(), "Error sizes mismatch");
    const auto* pBuildRanges = buildRanges_.data();

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.flags = buildFlags;
    buildInfo.type = type();
    buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure = accelerationStructure_;
    buildInfo.geometryCount = static_cast<uint32_t>(geometryData_.size());
    buildInfo.pGeometries = geometryData_.data();
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData.hostAddress = scratchData;
    VKW_CHECK_VK_FAIL(
        device_->vk().vkBuildAccelerationStructuresKHR(
            device_->getHandle(), VK_NULL_HANDLE, 1, &buildInfo, &pBuildRanges),
        "Error building BLAS on host");
}
} // namespace vkw