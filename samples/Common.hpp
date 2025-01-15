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

#include <vkWrappers/wrappers.hpp>
#include <vulkan/vulkan.h>

namespace vkw
{
struct BufferPropertyFlags
{
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memoryFlags;
};
struct ImagePropertyFlags
{
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags memoryFlags;
};
} // namespace vkw

// -----------------------------------------------------------------------------

static constexpr vkw::BufferPropertyFlags hostStagingFlags
    = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

static constexpr vkw::BufferPropertyFlags deviceFlags
    = {VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
           | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

static constexpr vkw::BufferPropertyFlags uniformDeviceFlags
    = {VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

static constexpr vkw::BufferPropertyFlags uniformHostStagingFlags
    = {VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
           | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

static constexpr vkw::ImagePropertyFlags imgDeviceFlags = {
    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

static constexpr vkw::BufferPropertyFlags vertexBufferFlags
    = {VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

static constexpr vkw::BufferPropertyFlags indexBufferFlags
    = {VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

// -----------------------------------------------------------------------------

template <typename T>
static inline T randVal()
{
    return 2.0f * float(rand()) / float(RAND_MAX) - 1.0f;
}

template <typename T>
static std::vector<T> randArray(const size_t size)
{
    std::vector<T> ret(size);
    for(size_t i = 0; i < size; i++)
    {
        ret[i] = randVal<T>();
    }
    return ret;
}

template <typename T>
static bool compareArrays(std::vector<T>& v0, std::vector<T>& v1)
{
    assert(v0.size() == v1.size());
    for(size_t i = 0; i < v0.size(); i++)
    {
        if(v0[i] != v1[i])
        {
            return false;
        }
    }
    return true;
}

template <typename T>
void uploadData(vkw::Device& device, const T* srcPtr, vkw::DeviceBuffer<T>& dst)
{
    vkw::HostStagingBuffer<T> stagingBuffer(
        device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, dst.size());
    stagingBuffer.copyFromHost(srcPtr, dst.size());

    vkw::Queue transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];
    vkw::CommandPool cmdPool(device, transferQueue);
    auto cmdBuffer = cmdPool.createCommandBuffer();

    vkw::Fence fence(device);
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.copyBuffer(stagingBuffer, dst);
    cmdBuffer.end();

    transferQueue.submit(cmdBuffer, fence);
    fence.wait();
}

template <typename T>
void downloadData(vkw::Device& device, const vkw::DeviceBuffer<T>& src, T* dstPtr)
{
    vkw::HostStagingBuffer<T> stagingBuffer(
        device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, src.size());

    vkw::Queue transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];
    vkw::CommandPool cmdPool(device, transferQueue);
    auto cmdBuffer = cmdPool.createCommandBuffer();

    vkw::Fence fence(device);
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.copyBuffer(src, stagingBuffer);
    cmdBuffer.end();

    transferQueue.submit(cmdBuffer, fence);
    fence.wait();

    stagingBuffer.copyToHost(dstPtr, src.size());
}