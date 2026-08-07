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

#include "Utils.hpp"

bool isFormatSupported(
    const VkPhysicalDevice physicalDevice, const VkFormat format, const VkFormatFeatureFlags requiredFeatures,
    const VkImageTiling tiling)
{
    VkFormatProperties properties = {};
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
    const auto features
        = (tiling == VK_IMAGE_TILING_LINEAR) ? properties.linearTilingFeatures : properties.optimalTilingFeatures;
    return (features & requiredFeatures) == requiredFeatures;
}

bool isImageSupported(
    const VkPhysicalDevice physicalDevice, const VkFormat format, const VkImageUsageFlags usage,
    const VkImageTiling tiling)
{
    VkImageFormatProperties properties = {};
    const VkResult result = vkGetPhysicalDeviceImageFormatProperties(
        physicalDevice, format, VK_IMAGE_TYPE_2D, tiling, usage, 0, &properties);
    return result == VK_SUCCESS;
}

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