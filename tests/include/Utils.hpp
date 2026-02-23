/*
 * Copyright (c) 2026 Adrien ARNAUD
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

bool changeImageLayout(
    const vkw::Device& device, const vkw::BaseImage& image, const VkImageLayout srcLayout,
    const VkImageLayout dstLayout)
{
    auto initQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, initQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        vkw::createImageMemoryBarrier(image, 0, 0, srcLayout, dstLayout));
    cmdBuffer.end();

    vkw::Fence initFence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(initFence.initialized());

    VKW_CHECK_VK_RETURN_FALSE(initQueue.submit(cmdBuffer, initFence));
    VKW_CHECK_BOOL_RETURN_FALSE(initFence.wait());

    return true;
}

template <typename SrcBufferType>
bool uploadBuffer(const vkw::Device& device, const void* src, SrcBufferType& dst, const size_t count)
{
    using T = typename SrcBufferType::value_type;

    if(dst.hostVisible()) { return dst.copyFromHost(src, count); }
    else
    {
        vkw::HostToDeviceBuffer<T> stagingBuffer{device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
        VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.initialized());
        VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.copyFromHost(src, count));

        auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

        vkw::CommandPool cmdPool{device, transferQueue};
        VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

        auto cmdBuffer = cmdPool.createCommandBuffer();
        VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

        cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        cmdBuffer.copyBuffer(stagingBuffer, dst);
        cmdBuffer.end();

        vkw::Fence transferFence{device};
        VKW_CHECK_BOOL_RETURN_FALSE(transferFence.initialized());

        VKW_CHECK_VK_RETURN_FALSE(transferQueue.submit(cmdBuffer, transferFence));
        VKW_CHECK_BOOL_RETURN_FALSE(transferFence.wait());
    }

    return true;
}

template <typename SrcBufferType>
bool downloadBuffer(const vkw::Device& device, const SrcBufferType& src, void* dst, const size_t count)
{
    using T = typename SrcBufferType::value_type;

    if(src.hostVisible()) { return src.copyToHost(dst, count); }
    else
    {
        vkw::HostToDeviceBuffer<T> stagingBuffer{device, count, VK_BUFFER_USAGE_TRANSFER_DST_BIT};
        VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.initialized());

        auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

        vkw::CommandPool cmdPool{device, transferQueue};
        VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

        auto cmdBuffer = cmdPool.createCommandBuffer();
        VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

        cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        cmdBuffer.copyBuffer(src, stagingBuffer);
        cmdBuffer.end();

        vkw::Fence transferFence{device};
        VKW_CHECK_BOOL_RETURN_FALSE(transferFence.initialized());

        VKW_CHECK_VK_RETURN_FALSE(transferQueue.submit(cmdBuffer, transferFence));
        VKW_CHECK_BOOL_RETURN_FALSE(transferFence.wait());

        VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.copyToHost(dst, count));
    }

    return true;
}

template <typename T, typename DstImageType>
bool uploadImage(
    const vkw::Device& device, const void* src, DstImageType& dst, const uint32_t w, const uint32_t h)
{
    const uint32_t res = w * h;

    vkw::DeviceToHostBuffer<T> stagingBuffer{device, res, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.initialized());
    VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.copyFromHost(src, res));

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = w;
    copyRegion.bufferImageHeight = h;
    copyRegion.imageOffset = VkOffset3D{0, 0, 0};
    copyRegion.imageExtent = {w, h, 1};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageSubresource.mipLevel = 0;
    cmdBuffer.copyBufferToImage(stagingBuffer, dst, VK_IMAGE_LAYOUT_GENERAL, {copyRegion});
    cmdBuffer.end();

    vkw::Fence transferFence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(transferFence.initialized());

    VKW_CHECK_VK_RETURN_FALSE(transferQueue.submit(cmdBuffer, transferFence));
    VKW_CHECK_BOOL_RETURN_FALSE(transferFence.wait());

    return true;
}

template <typename T, typename SrcImageType>
bool downloadImage(
    const vkw::Device& device, const SrcImageType& src, void* dst, const uint32_t w, const uint32_t h)
{
    const uint32_t res = w * h;

    vkw::DeviceToHostBuffer<T> stagingBuffer{device, res, VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.initialized());

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = w;
    copyRegion.bufferImageHeight = h;
    copyRegion.imageOffset = VkOffset3D{0, 0, 0};
    copyRegion.imageExtent = {w, h, 1};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageSubresource.mipLevel = 0;
    cmdBuffer.copyImageToBuffer(src, VK_IMAGE_LAYOUT_GENERAL, stagingBuffer, copyRegion);
    cmdBuffer.end();

    vkw::Fence transferFence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(transferFence.initialized());

    VKW_CHECK_VK_RETURN_FALSE(transferQueue.submit(cmdBuffer, transferFence));
    VKW_CHECK_BOOL_RETURN_FALSE(transferFence.wait());

    VKW_CHECK_BOOL_RETURN_FALSE(stagingBuffer.copyToHost(dst, res));

    return true;
}
