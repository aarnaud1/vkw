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

#include "vkWrappers/wrappers/BottomLevelAccelerationStructure.hpp"

namespace vkw
{
BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(
    Device& device, const bool buildOnHost)
    : BaseAccelerationStructure()
{
    VKW_CHECK_BOOL_THROW(
        this->init(device, buildOnHost), "Error creating bottom level acceleration structure");
}

BottomLevelAccelerationStructure& BottomLevelAccelerationStructure::operator=(
    BottomLevelAccelerationStructure&& rhs)
{
    this->clear();

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
    this->initializeBuildSizes(buildFlags);
    VKW_CHECK_BOOL_THROW(
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
    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateAccelerationStructureKHR(
            device_->getHandle(), &createInfo, nullptr, &accelerationStructure_),
        "Error creating BLAS");

    for(size_t geometryId = 0; geometryId < geometryData_.size(); ++geometryId)
    {
        const auto& rangeList = buildRanges_[geometryId];
        if(rangeList.empty())
        {
            continue;
        }

        for(const auto& buildRange : rangeList)
        {
            ppGeometries_.push_back(geometryData_.data() + geometryId);
            ppBuildRanges_.push_back(&buildRange);
        }
    }
}

void BottomLevelAccelerationStructure::clear()
{
    ppGeometries_.clear();
    ppBuildRanges_.clear();

    buildOnHost_ = false;

    buildRanges_.clear();
    geometryData_.clear();
    geometryType_ = GeometryType::Undefined;

    primitiveCounts_.clear();
    VKW_DELETE_VK(AccelerationStructureKHR, accelerationStructure_);
    updateSizes_ = {};
    buildSizes_ = {};
    type_ = {};

    storageBuffer_.clear();

    device_ = nullptr;

    initialized_ = false;
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
    buildRanges_.emplace_back();

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
    buildRanges_.emplace_back();

    return *this;
}

void BottomLevelAccelerationStructure::build(
    void* scratchData, const VkBuildAccelerationStructureFlagsKHR buildFlags)
{
    VKW_CHECK_BOOL_THROW(buildOnHost_, "Error BLAS not mean to be built on host");

    if(ppGeometries_.size() != ppBuildRanges_.size())
    {
        throw(std::runtime_error("Ranges mismatch"));
    }

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
    buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.pNext = nullptr;
    buildInfo.flags = buildFlags;
    buildInfo.type = type();
    buildInfo.geometryCount = static_cast<uint32_t>(ppGeometries_.size());
    buildInfo.pGeometries = nullptr;
    buildInfo.ppGeometries = ppGeometries_.data();
    buildInfo.scratchData.hostAddress = scratchData;

    device_->vk().vkBuildAccelerationStructuresKHR(
        device_->getHandle(), VK_NULL_HANDLE, 1, &buildInfo, ppBuildRanges_.data());
}
} // namespace vkw