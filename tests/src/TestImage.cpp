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

#include "TestImage.hpp"

#include "Utils.hpp"

#include <array>
#include <vector>
#include <vkw/vkw.hpp>

static const char* testName = "ImageTest";

struct ImageSize
{
    uint32_t w;
    uint32_t h;
};

static constexpr std::array<ImageSize, 3> imageSizes
    = {ImageSize{1, 1}, ImageSize{17, 13}, ImageSize{256, 256}};
static constexpr std::array<VkFormat, 2> imageFormats = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM};
static constexpr std::array<VkImageUsageFlags, 4> imageUsages = {
    VK_IMAGE_USAGE_STORAGE_BIT, VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};

// -----------------------------------------------------------------------------------------------------------

static bool testImageCreation(const vkw::Device& device, const VkPhysicalDevice physicalDevice);
static bool testImageCreationLinearTiling(const vkw::Device& device, const VkPhysicalDevice physicalDevice);
static bool testImageMove(const vkw::Device& device);
static bool testImageViewCreation(const vkw::Device& device);
static bool testImageLayoutTransition(const vkw::Device& device);
static bool testImageUploadDownload(const vkw::Device& device);
static bool testImagePartialCopy(const vkw::Device& device);
static bool testImageBlit(const vkw::Device& device);

// -----------------------------------------------------------------------------------------------------------

bool launchImageTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    if(!TestUtils::isFormatSupported(
           physicalDevice, VK_FORMAT_R32_SFLOAT,
           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT
               | VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
    {
        vkw::utils::Log::Info(testName, "R32_SFLOAT not fully supported, skipping");
        return true;
    }
    if(!TestUtils::isFormatSupported(
           physicalDevice, VK_FORMAT_R8G8B8A8_UNORM,
           VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT
               | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
    {
        vkw::utils::Log::Info(testName, "R8G8B8A8_UNORM not fully supported, skipping");
        return true;
    }

    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(device.init(instance, physicalDevice, {}, {}));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    vkw::utils::Log::Info(testName, "Checking image creation...");
    if(!testImageCreation(device, physicalDevice))
    {
        vkw::utils::Log::Warning(testName, "  Creation - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking image creation with linear tiling...");
    if(!testImageCreationLinearTiling(device, physicalDevice))
    {
        vkw::utils::Log::Warning(testName, "  Creation (linear tiling) - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking image move semantics...");
    if(!testImageMove(device))
    {
        vkw::utils::Log::Warning(testName, "  Move semantics - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking image view creation...");
    if(!testImageViewCreation(device))
    {
        vkw::utils::Log::Warning(testName, "  ImageView creation - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking image layout transition...");
    if(!testImageLayoutTransition(device))
    {
        vkw::utils::Log::Warning(testName, "  Layout transition - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking image upload/download...");
    if(!testImageUploadDownload(device))
    {
        vkw::utils::Log::Warning(testName, "  Upload/download - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking partial image copy...");
    if(!testImagePartialCopy(device))
    {
        vkw::utils::Log::Warning(testName, "  Partial copy - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking image blit...");
    if(!testImageBlit(device))
    {
        vkw::utils::Log::Warning(testName, "  Blit - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return failedTests == 0;
}

// -----------------------------------------------------------------------------------------------------------

template <vkw::MemoryType memType>
static bool testImageCreationForMemType(
    const vkw::Device& device, const VkPhysicalDevice physicalDevice, const char* memTypeName)
{
    using Flags = vkw::MemoryFlags<memType>;

    bool ret = true;
    for(const auto size : imageSizes)
    {
        for(const auto format : imageFormats)
        {
            for(const auto usage : imageUsages)
            {
                if(!TestUtils::isImageSupported(physicalDevice, format, usage))
                {
                    continue;
                }

                const VkExtent3D extent{size.w, size.h, 1};
                vkw::Image<memType> image{
                    device, VK_IMAGE_TYPE_2D,        format, extent, usage, VK_SAMPLE_COUNT_1_BIT,
                    1,      VK_IMAGE_TILING_OPTIMAL, 1,      0};
                if(!image.initialized())
                {
                    vkw::utils::Log::Error(
                        testName, "  [%s] init failed (w=%u, h=%u, format=%d, usage=%u)", memTypeName, size.w,
                        size.h, static_cast<int>(format), static_cast<uint32_t>(usage));
                    ret = false;
                    continue;
                }

                const auto imgExtent = image.extent();
                if((imgExtent.width != size.w) || (imgExtent.height != size.h) || (imgExtent.depth != 1))
                {
                    vkw::utils::Log::Error(
                        testName, "  [%s] extent mismatch (w=%u, h=%u, format=%d)", memTypeName, size.w,
                        size.h, static_cast<int>(format));
                    ret = false;
                }

                if(image.format() != format)
                {
                    vkw::utils::Log::Error(
                        testName, "  [%s] format mismatch (w=%u, h=%u, format=%d)", memTypeName, size.w,
                        size.h, static_cast<int>(format));
                    ret = false;
                }

                if((Flags::requiredFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && !image.deviceLocal())
                {
                    vkw::utils::Log::Error(
                        testName, "  [%s] expected deviceLocal (w=%u, h=%u, format=%d)", memTypeName, size.w,
                        size.h, static_cast<int>(format));
                    ret = false;
                }
                if((Flags::requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && !image.hostVisible())
                {
                    vkw::utils::Log::Error(
                        testName, "  [%s] expected hostVisible (w=%u, h=%u, format=%d)", memTypeName, size.w,
                        size.h, static_cast<int>(format));
                    ret = false;
                }
            }
        }
    }
    return ret;
}

bool testImageCreation(const vkw::Device& device, const VkPhysicalDevice physicalDevice)
{
    bool ret = true;
    ret &= testImageCreationForMemType<vkw::MemoryType::Device>(device, physicalDevice, "Device");
    return ret;
}

// -----------------------------------------------------------------------------------------------------------

bool testImageCreationLinearTiling(const vkw::Device& device, const VkPhysicalDevice physicalDevice)
{
    bool ret = true;
    for(const auto size : imageSizes)
    {
        for(const auto format : imageFormats)
        {
            const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if(!TestUtils::isImageSupported(physicalDevice, format, usage, VK_IMAGE_TILING_LINEAR))
            {
                continue;
            }

            const VkExtent3D extent{size.w, size.h, 1};
            vkw::HostImage<> image{
                device, VK_IMAGE_TYPE_2D,       format, extent, usage, VK_SAMPLE_COUNT_1_BIT,
                1,      VK_IMAGE_TILING_LINEAR, 1,      0};
            if(!image.initialized())
            {
                vkw::utils::Log::Error(
                    testName, "  [LinearTiling] init failed (w=%u, h=%u, format=%d)", size.w, size.h,
                    static_cast<int>(format));
                ret = false;
                continue;
            }

            if(!image.hostVisible())
            {
                vkw::utils::Log::Error(
                    testName, "  [LinearTiling] expected hostVisible (w=%u, h=%u, format=%d)", size.w, size.h,
                    static_cast<int>(format));
                ret = false;
            }

            const auto imgExtent = image.extent();
            if((imgExtent.width != size.w) || (imgExtent.height != size.h) || (imgExtent.depth != 1))
            {
                vkw::utils::Log::Error(
                    testName, "  [LinearTiling] extent mismatch (w=%u, h=%u, format=%d)", size.w, size.h,
                    static_cast<int>(format));
                ret = false;
            }
        }
    }
    return ret;
}

// -----------------------------------------------------------------------------------------------------------

template <vkw::MemoryType memType>
static bool testImageMoveForMemType(const vkw::Device& device, const char* memTypeName)
{
    const VkExtent3D extent{64, 64, 1};

    vkw::Image<memType> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if(!image.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] init failed", memTypeName);
        return false;
    }

    const auto handle = image.getHandle();

    vkw::Image<memType> moved{std::move(image)};
    if(!moved.initialized() || (moved.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  [%s] move-construct failed", memTypeName);
        return false;
    }
    if(image.initialized() || (image.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  [%s] moved-from object not cleared", memTypeName);
        return false;
    }

    vkw::Image<memType> assigned{};
    assigned = std::move(moved);
    if(!assigned.initialized() || (assigned.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  [%s] move-assign failed", memTypeName);
        return false;
    }
    if(moved.initialized() || (moved.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  [%s] moved-from object not cleared after assign", memTypeName);
        return false;
    }

    return true;
}

bool testImageMove(const vkw::Device& device)
{
    bool ret = true;
    ret &= testImageMoveForMemType<vkw::MemoryType::Device>(device, "Device");
    return ret;
}

// -----------------------------------------------------------------------------------------------------------

bool testImageViewCreation(const vkw::Device& device)
{
    const VkExtent3D extent{64, 64, 1};

    vkw::DeviceImage<> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if(!image.initialized())
    {
        vkw::utils::Log::Error(testName, "  ImageView test: image init failed");
        return false;
    }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;

    vkw::ImageView imageView{device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange};
    if(!imageView.initialized())
    {
        vkw::utils::Log::Error(testName, "  ImageView test: view init failed");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testImageLayoutTransition(const vkw::Device& device)
{
    const VkExtent3D extent{64, 64, 1};

    vkw::DeviceImage<> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if(!image.initialized())
    {
        vkw::utils::Log::Error(testName, "  Layout transition: image init failed");
        return false;
    }

    if(!TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        vkw::utils::Log::Error(testName, "  Layout transition failed");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testImageUploadDownload(const vkw::Device& device)
{
    static constexpr uint32_t w = 17;
    static constexpr uint32_t h = 13;
    static constexpr size_t count = w * h;

    std::vector<float> pattern(count);
    TestUtils::fillPattern<float>(pattern.data(), count);

    const VkExtent3D extent{w, h, 1};
    vkw::HostDeviceImage<> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if(!image.initialized())
    {
        vkw::utils::Log::Error(testName, "  Upload/download: image init failed");
        return false;
    }

    if(!TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        vkw::utils::Log::Error(testName, "  Upload/download: layout transition failed");
        return false;
    }

    if(!TestUtils::uploadImage<float>(device, pattern.data(), image, w, h))
    {
        vkw::utils::Log::Error(testName, "  Upload/download: upload failed");
        return false;
    }

    std::vector<float> result(count);
    if(!TestUtils::downloadImage<float>(device, image, result.data(), w, h))
    {
        vkw::utils::Log::Error(testName, "  Upload/download: download failed");
        return false;
    }

    if(!TestUtils::compareData(pattern.data(), result.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Upload/download: content mismatch");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testImagePartialCopy(const vkw::Device& device)
{
    static constexpr uint32_t w = 64;
    static constexpr uint32_t h = 64;
    static constexpr size_t count = w * h;

    static constexpr uint32_t subX = 16;
    static constexpr uint32_t subY = 16;
    static constexpr uint32_t subW = 32;
    static constexpr uint32_t subH = 32;

    std::vector<float> pattern(count);
    TestUtils::fillPattern<float>(pattern.data(), count);

    std::vector<float> zeros(count, 0.0f);

    const VkExtent3D extent{w, h, 1};
    vkw::HostDeviceImage<> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if(!image.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy: image init failed");
        return false;
    }

    if(!TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        vkw::utils::Log::Error(testName, "  Partial copy: layout transition failed");
        return false;
    }

    if(!TestUtils::uploadImage<float>(device, zeros.data(), image, w, h))
    {
        vkw::utils::Log::Error(testName, "  Partial copy: initial clear upload failed");
        return false;
    }

    vkw::DeviceUploadBuffer<float> stagingBuffer{device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    if(!stagingBuffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy: staging buffer init failed");
        return false;
    }
    if(!stagingBuffer.copyFromHost(pattern.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Partial copy: staging buffer copyFromHost failed");
        return false;
    }

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    if(!cmdPool.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy: command pool init failed");
        return false;
    }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy: command buffer init failed");
        return false;
    }

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = (subY * w + subX) * sizeof(float);
    copyRegion.bufferRowLength = w;
    copyRegion.bufferImageHeight = h;
    copyRegion.imageOffset = VkOffset3D{static_cast<int32_t>(subX), static_cast<int32_t>(subY), 0};
    copyRegion.imageExtent = {subW, subH, 1};
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageSubresource.mipLevel = 0;

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.copyBufferToImage(stagingBuffer, image, VK_IMAGE_LAYOUT_GENERAL, copyRegion);
    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createImageMemoryBarrier(
            image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy: fence init failed");
        return false;
    }
    if(transferQueue.submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        vkw::utils::Log::Error(testName, "  Partial copy: submit failed");
        return false;
    }
    if(!fence.wait())
    {
        vkw::utils::Log::Error(testName, "  Partial copy: fence wait failed");
        return false;
    }

    std::vector<float> result(count);
    if(!TestUtils::downloadImage<float>(device, image, result.data(), w, h))
    {
        vkw::utils::Log::Error(testName, "  Partial copy: download failed");
        return false;
    }

    for(uint32_t y = 0; y < h; ++y)
    {
        for(uint32_t x = 0; x < w; ++x)
        {
            const size_t idx = y * w + x;
            const bool inSubRegion = (x >= subX) && (x < subX + subW) && (y >= subY) && (y < subY + subH);
            const float expected = inSubRegion ? pattern[idx] : 0.0f;
            if(result[idx] != expected)
            {
                vkw::utils::Log::Error(testName, "  Partial copy: content mismatch at (%u, %u)", x, y);
                return false;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testImageBlit(const vkw::Device& device)
{
    static constexpr uint32_t w = 64;
    static constexpr uint32_t h = 64;
    static constexpr size_t count = w * h;

    std::vector<float> pattern(count);
    TestUtils::fillPattern<float>(pattern.data(), count);

    const VkExtent3D extent{w, h, 1};
    vkw::HostDeviceImage<> src{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    vkw::HostDeviceImage<> dst{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT};
    if(!src.initialized() || !dst.initialized())
    {
        vkw::utils::Log::Error(testName, "  Blit: image init failed");
        return false;
    }

    if(!TestUtils::changeImageLayout(device, src, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        vkw::utils::Log::Error(testName, "  Blit: src layout transition failed");
        return false;
    }
    if(!TestUtils::changeImageLayout(device, dst, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        vkw::utils::Log::Error(testName, "  Blit: dst layout transition failed");
        return false;
    }

    if(!TestUtils::uploadImage<float>(device, pattern.data(), src, w, h))
    {
        vkw::utils::Log::Error(testName, "  Blit: upload failed");
        return false;
    }

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    if(!cmdPool.initialized())
    {
        vkw::utils::Log::Error(testName, "  Blit: command pool init failed");
        return false;
    }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  Blit: command buffer init failed");
        return false;
    }

    VkImageBlit blitRegion = {};
    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcOffsets[0] = VkOffset3D{0, 0, 0};
    blitRegion.srcOffsets[1] = VkOffset3D{static_cast<int32_t>(w), static_cast<int32_t>(h), 1};
    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstOffsets[0] = VkOffset3D{0, 0, 0};
    blitRegion.dstOffsets[1] = VkOffset3D{static_cast<int32_t>(w), static_cast<int32_t>(h), 1};

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.blitImage(
        src, VK_IMAGE_LAYOUT_GENERAL, dst, VK_IMAGE_LAYOUT_GENERAL, blitRegion, VK_FILTER_NEAREST);
    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createImageMemoryBarrier(
            dst, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized())
    {
        vkw::utils::Log::Error(testName, "  Blit: fence init failed");
        return false;
    }
    if(transferQueue.submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        vkw::utils::Log::Error(testName, "  Blit: submit failed");
        return false;
    }
    if(!fence.wait())
    {
        vkw::utils::Log::Error(testName, "  Blit: fence wait failed");
        return false;
    }

    std::vector<float> result(count);
    if(!TestUtils::downloadImage<float>(device, dst, result.data(), w, h))
    {
        vkw::utils::Log::Error(testName, "  Blit: download failed");
        return false;
    }

    if(!TestUtils::compareData(pattern.data(), result.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Blit: content mismatch");
        return false;
    }

    return true;
}