/*
 * Copyright (C) 2024 Adrien ARNAUD
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

#include "vkWrappers/wrappers/Memory.hpp"

#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
Memory::Memory(Device& device, VkMemoryPropertyFlags properties)
{
    VKW_CHECK_BOOL_THROW(this->init(device, properties), "Initializing memory object");
}

Memory::Memory(Memory&& cp) { *this = std::move(cp); }

Memory& Memory::operator=(Memory&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);

    std::swap(allocatedSize_, cp.allocatedSize_);
    std::swap(propertyFlags_, cp.propertyFlags_);
    std::swap(memory_, cp.memory_);

    std::swap(nextOffset_, cp.nextOffset_);
    std::swap(offsets_, cp.offsets_);
    std::swap(memObjects_, cp.memObjects_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

Memory::~Memory() { this->clear(); }

bool Memory::init(Device& device, VkMemoryPropertyFlags properties)
{
    if(!initialized_)
    {
        device_ = &device;
        propertyFlags_ = properties;
        memory_ = VK_NULL_HANDLE;

        initialized_ = true;
    }

    return true;
}

void Memory::clear()
{
    release();

    for(auto& memObject : memObjects_)
    {
        memObject->clear();
    }
    memObjects_.clear();
    offsets_.clear();
    nextOffset_ = 0;

    device_ = nullptr;

    allocatedSize_ = {};
    propertyFlags_ = {};
    memory_ = VK_NULL_HANDLE;

    initialized_ = false;
}

bool Memory::allocate()
{
    const size_t objCount = memObjects_.size();
    if(objCount == 0)
    {
        utils::Log::Error("vkw::Memory", "Allocating empty memory object");
        return false;
    }

    const VkDeviceSize requiredSize = offsets_.back() + memObjects_.back()->memSize_;
    const uint32_t memIndex = findMemoryType(propertyFlags_);
    if(memIndex == ~uint32_t(0))
    {
        utils::Log::Error("vkw", "No suitable memory found");
        return false;
    }

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.memoryTypeIndex = memIndex;
    allocateInfo.allocationSize = requiredSize;
    VKW_INIT_CHECK_VK(vkAllocateMemory(device_->getHandle(), &allocateInfo, nullptr, &memory_));

    // Bind resources
    for(size_t i = 0; i < objCount; ++i)
    {
        memObjects_[i]->memOffset_ = offsets_[i];
        VKW_INIT_CHECK_BOOL(memObjects_[i]->bindResource(memory_, offsets_[i]));
    }

    return true;
}

void Memory::release() { VKW_FREE_VK(Memory, memory_); }

uint32_t Memory::findMemoryType(VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device_->getPhysicalDevice(), &memProperties);

    // Finds a memory with exactly the required properties
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if(memProperties.memoryTypes[i].propertyFlags == properties)
        {
            return i;
        }
    }

    // Fallback to a memory property that contains the required flags
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    utils::Log::Error("vkw::Memory", "Memory properties not found");
    return ~uint32_t(0);
}
} // namespace vkw