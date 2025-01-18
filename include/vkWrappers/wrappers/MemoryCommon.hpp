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

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vkw
{
enum class MemoryType
{
    Device,
    Host,
    HostStaging,
    TransferHostDevice,
    TransferDeviceHost
};

template <MemoryType memType>
struct MemoryFlags
{
    static constexpr VkMemoryPropertyFlags requiredFlags = {};
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VmaMemoryUsage usage = {};
    static constexpr VmaAllocationCreateFlags allocationFlags = {};

    static constexpr bool hostVisible = false;
};
template <>
struct MemoryFlags<MemoryType::Device>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    static constexpr VmaAllocationCreateFlags allocationFlags = {};

    static constexpr bool hostVisible = false;
};
template <>
struct MemoryFlags<MemoryType::Host>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    static constexpr VmaAllocationCreateFlags allocationFlags
        = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

    static constexpr bool hostVisible = true;
};
template <>
struct MemoryFlags<MemoryType::HostStaging>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO;
    static constexpr VmaAllocationCreateFlags allocationFlags
        = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

    static constexpr bool hostVisible = true;
};
template <>
struct MemoryFlags<MemoryType::TransferHostDevice>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    static constexpr VmaAllocationCreateFlags allocationFlags
        = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    static constexpr bool hostVisible = true;
};
template <>
struct MemoryFlags<MemoryType::TransferDeviceHost>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    static constexpr VmaAllocationCreateFlags allocationFlags
        = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    static constexpr bool hostVisible = true;
};
} // namespace vkw