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

#include <vkw/vkw.hpp>

static VkSampleCountFlagBits getMaxSampleCount(
    const vkw::Device& device, const VkSampleCountFlags sampleCount)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &physicalDeviceProperties);

    const auto counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
                        & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    const auto mask = counts & ((sampleCount << 1) - 1);

    if(mask & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if(mask & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if(mask & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if(mask & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if(mask & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if(mask & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

template <typename T, typename DstBufferType>
static void uploadData(vkw::Device& device, const T* srcPtr, DstBufferType& dst)
{
    vkw::HostStagingBuffer<T> stagingBuffer(
        device, dst.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
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

template <typename T, typename DstBufferType>
static void downloadData(vkw::Device& device, const DstBufferType& src, T* dstPtr)
{
    vkw::HostStagingBuffer<T> stagingBuffer(
        device, src.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

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

template <typename... Args>
static VkPhysicalDevice findCompatibleDevice(
    const vkw::Instance& instance,
    const std::vector<const char*>& requiredExtensions,
    Args&&... args)
{
    const auto compatibleDevices = vkw::Device::listSupportedDevices(
        instance, requiredExtensions, {}, std::forward<Args>(args)...);

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    if(compatibleDevices.size() == 0)
    {
        return physicalDevice;
    }

    for(const auto& device : compatibleDevices)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            physicalDevice = device;
            break;
        }
    }
    if(physicalDevice == VK_NULL_HANDLE)
    {
        for(const auto& device : compatibleDevices)
        {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                physicalDevice = device;
                break;
            }
        }
    }

    return (physicalDevice == VK_NULL_HANDLE) ? compatibleDevices[0] : physicalDevice;
}