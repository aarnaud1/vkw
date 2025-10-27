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

#pragma once

#include "vkw/detail/Common.hpp"

#ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#    pragma GCC diagnostic ignored "-Wunused-variable"
#    pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wnullability-completeness"
#endif
#include <vk_mem_alloc.h>
#ifdef __clang__
#    pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#    pragma GCC diagnostic pop
#endif

namespace vkw
{
enum class MemoryType
{
    Device,             ///< Used for data only accessed from the device
    Host,               ///< Used for data that should be mapped on host
    HostStaging,        ///< Used for staging or uniform buffers, permanently mapped
    HostDevice,         ///< Used for large buffers that can be on host if device size is limited
    TransferHostDevice, ///< Used to upload data. Needs to be mapped before using
    TransferDeviceHost  ///< Used for readback. Needs to be mapped before using
};

template <MemoryType memType>
struct MemoryFlags
{
    static constexpr VkMemoryPropertyFlags requiredFlags = {};
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VmaMemoryUsage usage = {};
    static constexpr VmaAllocationCreateFlags allocationFlags = {};
};
template <>
struct MemoryFlags<MemoryType::Device>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    static constexpr VmaAllocationCreateFlags allocationFlags = {};
};
template <>
struct MemoryFlags<MemoryType::Host>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    static constexpr VmaAllocationCreateFlags allocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
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
};
template <>
struct MemoryFlags<MemoryType::HostDevice>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = {};
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO;
    static constexpr VmaAllocationCreateFlags allocationFlags = {};
};
template <>
struct MemoryFlags<MemoryType::TransferHostDevice>
{
    static constexpr VkMemoryPropertyFlags requiredFlags
        = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = {};
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    static constexpr VmaAllocationCreateFlags allocationFlags
        = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
};
template <>
struct MemoryFlags<MemoryType::TransferDeviceHost>
{
    static constexpr VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    static constexpr VkMemoryPropertyFlags preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    static constexpr VmaMemoryUsage usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    static constexpr VmaAllocationCreateFlags allocationFlags
        = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
};
} // namespace vkw