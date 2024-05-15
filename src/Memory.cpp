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

namespace vk
{
Memory::Memory(Device &device, VkMemoryPropertyFlags properties, bool external)
    : device_(device), properties_(properties), external_(external)
{
    memory_ = VK_NULL_HANDLE;
}

Memory::~Memory() { release(); }

void Memory::allocate()
{
    size_ = computeSize();

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = size_;
    allocateInfo.memoryTypeIndex = findMemoryType(properties_);
    if(external_)
    {
        VkExportMemoryAllocateInfo externalAllocateInfo = {};
        externalAllocateInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
        externalAllocateInfo.pNext = nullptr;
        externalAllocateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
        allocateInfo.pNext = &externalAllocateInfo;

        CHECK_VK(
            vkAllocateMemory(device_.getHandle(), &allocateInfo, nullptr, &memory_),
            "Allocating memory");
    }
    else
    {
        allocateInfo.pNext = nullptr;

        CHECK_VK(
            vkAllocateMemory(device_.getHandle(), &allocateInfo, nullptr, &memory_),
            "Allocating memory");
    }

    // Bind resources
    size_t offset = 0;
    for(auto &ptr : managedObjects_)
    {
        ptr->bindResource(getHandle(), offset);
        offset += ptr->getMemRequirements().size;
    }
}

void Memory::release()
{
    if(memory_ != VK_NULL_HANDLE)
    {
        vkFreeMemory(device_.getHandle(), memory_, nullptr);
        memory_ = VK_NULL_HANDLE;
    }
}

void Memory::clear()
{
    managedObjects_.clear();
    release();
}

int Memory::getExternalMemHandle()
{
    int fd = -1;
    VkMemoryGetFdInfoKHR getFdInfo = {};
    getFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    getFdInfo.pNext = nullptr;
    getFdInfo.memory = memory_;
    getFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

    PFN_vkGetMemoryFdKHR fpGetMemoryFdKHR;
    fpGetMemoryFdKHR
        = (PFN_vkGetMemoryFdKHR) vkGetDeviceProcAddr(device_.getHandle(), "vkGetMemoryFdKHR");
    if(!fpGetMemoryFdKHR)
    {
        fprintf(stderr, "Error : vkGetMemoryFdKHR unavailable\n");
        exit(1);
    }

    if(fpGetMemoryFdKHR(device_.getHandle(), &getFdInfo, &fd) != VK_SUCCESS)
    {
        fprintf(stdout, "Error gettng memory file descriptor\n");
        exit(1);
    }

    return fd;
}

uint32_t Memory::findMemoryType(VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device_.getPhysicalDevice(), &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    fprintf(stderr, "Error, could not find any suitable memory type\n");
    exit(1);
}

uint32_t Memory::computeSize()
{
    uint32_t ret = 0;
    for(auto &ptr : managedObjects_)
    {
        ret += ptr->getMemRequirements().size;
    }
    return ret;
}
} // namespace vk
