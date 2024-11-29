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
/// @brief Enum value to specify memory usages for a Memory object.
enum class MemoryType
{
    /// Device memory type that can be used for image storage.
    Device, /// Guarantees DEVICE_LOCAL, prefers not HOST_VISIBLE

    /// Host memory types, can not be used for image storage.
    Host,               /// Guarantees HOST_VISIBLE, HOST_COHERENT, prefers not DEVICE_LOCAL
                        /// Memory that resides on Host.
    HostStaging,        /// Guarantees HOST_VISIBLE, HOST_COHERENT, prefers DEVICE_LOCAL
                        /// Memory for staging buffers, always mapped
    TransferHostDevice, /// Guarantees HOST_VISIBLE, HOST_COHERENT,
                        /// Use it to transfer data from host to device.
    TransferDeviceHost  /// Guarentees HOST_VISIBLE, prefers HOST_CACHED, HOST_COHERENT
                        /// Use it to transfer data from device to host.
};

template <MemoryType memType>
struct MemoryFlags
{
    static constexpr VkMemoryPropertyFlags requiredFlags = {};
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VkMemoryPropertyFlags undesiredFlags = {};
    static constexpr bool hostVisible = false;
};
template <>
struct MemoryFlags<MemoryType::Device>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VkMemoryPropertyFlags undesiredFlags = {VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT};
    static constexpr bool hostVisible = false;
};
template <>
struct MemoryFlags<MemoryType::Host>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VkMemoryPropertyFlags undesiredFlags = {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
    static constexpr bool hostVisible = false;
};
template <>
struct MemoryFlags<MemoryType::HostStaging>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
    static constexpr VkMemoryPropertyFlags undesiredFlags = {};
    static constexpr bool hostVisible = true;
};
template <>
struct MemoryFlags<MemoryType::TransferHostDevice>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VkMemoryPropertyFlags undesiredFlags = {};
    static constexpr bool hostVisible = true;
};
template <>
struct MemoryFlags<MemoryType::TransferDeviceHost>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VkMemoryPropertyFlags undesiredFlags
        = {VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT};
    static constexpr bool hostVisible = true;
};
} // namespace vkw