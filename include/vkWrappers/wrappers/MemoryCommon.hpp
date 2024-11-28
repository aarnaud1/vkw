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

#pragma once

#include <vulkan/vulkan.h>

namespace vkw
{
enum class MemoryType
{
    /// Device local memory type
    Device, /// Guarantees DEVICE_LOCAL, prefers not HOST_VISIBLE

    /// Host memory types, permanently mapped
    HostStaging,  /// Guarantees HOST_VISIBLE, HOST_COHERENT, prefers DEVICE_LOCAL
    Host,         /// Guarantees HOST_VISIBLE, prefers not DEVICE_LOCAL
    HostToDevice, /// Guarantees HOST_VISIBLE, prefers HOST_COHERENT
    DeviceToHost  /// Guarantees HOST_VISIBLE, prefers HOST_COHERENT, HOST_CACHED
};

template <MemoryType = MemoryType::Device>
struct MemoryFlags
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = 0;
    static constexpr VkMemoryPropertyFlags undesiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr bool hostMapped = false;
};
template <>
struct MemoryFlags<MemoryType::HostStaging>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = 0;
    static constexpr VkMemoryPropertyFlags undesiredFlags = 0;
    static constexpr bool hostMapped = true;
};
template <>
struct MemoryFlags<MemoryType::Host>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = 0;
    static constexpr VkMemoryPropertyFlags undesiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr bool hostMapped = true;
};
template <>
struct MemoryFlags<MemoryType::HostToDevice>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags undesiredFlags = 0;
    static constexpr bool hostMapped = false;
};
template <>
struct MemoryFlags<MemoryType::DeviceToHost>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags
        = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    static constexpr VkMemoryPropertyFlags undesiredFlags = 0;
    static constexpr bool hostMapped = false;
};
} // namespace vkw