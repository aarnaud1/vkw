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

#include "TestDescriptorBuffers.hpp"

#include "Utils.hpp"

#include <memory>
#include <vector>
#include <vkw/high_level/Types.hpp>
#include <vkw/vkw.hpp>

static const char* testName = "DescriptorBuffersTest";

// -----------------------------------------------------------------------------------------------------------

static bool testStorageBufferDescriptorBuffer(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testStorageBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testStorageImageDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize);

static bool testStorageTexelBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testUniformBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount);

static bool testUniformTexelBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount);

static bool testSampledImageDescriptorBufferIndexing(const vkw::Device& device, const size_t descriptorCount);

static bool testCombinedImageSamplerDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount);

// -----------------------------------------------------------------------------------------------------------

static const uint32_t fillStorageBufferDescriptorBufferComp[] = {
#include "spv/FillStorageBufferDescriptorBuffer.comp.spv"
};
static const uint32_t fillStorageBuffersDescriptorBufferIndexingComp[] = {
#include "spv/FillStorageBuffersDescriptorBufferIndexing.comp.spv"
};
static const uint32_t updateStorageBuffersDescriptorBufferIndexingComp[] = {
#include "spv/UpdateStorageBuffersDescriptorBufferIndexing.comp.spv"
};

static const uint32_t fillStorageImagesDescriptorIndexingComp[] = {
#include "spv/FillStorageImagesDescriptorIndexing.comp.spv"
};
static const uint32_t updateStorageImagesDescriptorIndexingComp[] = {
#include "spv/UpdateStorageImagesDescriptorIndexing.comp.spv"
};

static const uint32_t fillStorageTexelBuffersDescriptorIndexingComp[] = {
#include "spv/FillStorageTexelBuffersDescriptorIndexing.comp.spv"
};
static const uint32_t updateStorageTexelBuffersDescriptorIndexingComp[] = {
#include "spv/UpdateStorageTexelBuffersDescriptorIndexing.comp.spv"
};

static const uint32_t readUniformBuffersDescriptorIndexingComp[] = {
#include "spv/ReadUniformBuffersDescriptorIndexing.comp.spv"
};

static const uint32_t readUniformTexelBuffersDescriptorIndexingComp[] = {
#include "spv/ReadUniformTexelBuffersDescriptorIndexing.comp.spv"
};

static const uint32_t readSampledImagesDescriptorBufferIndexingComp[] = {
#include "spv/ReadSampledImagesDescriptorBufferIndexing.comp.spv"
};

static const uint32_t readCombinedImageSamplersDescriptorIndexingComp[] = {
#include "spv/ReadCombinedImageSamplersDescriptorIndexing.comp.spv"
};

// -----------------------------------------------------------------------------------------------------------

static bool downloadAndCheckBuffers(
    const vkw::Device& device, const std::vector<vkw::DeviceBuffer<float>>& bufferList,
    const size_t bufferSize, const float startValue)
{
    auto bufferData = std::make_unique<float[]>(bufferSize);
    float expected = startValue;
    for(const auto& buffer : bufferList)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::downloadFromDeviceBuffer(device, buffer, bufferData.get(), bufferSize));
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::checkBufferContents<float>(bufferData.get(), expected, bufferSize, 1));
        expected += 1.0f;
    }
    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool launchDescriptorBuffersTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extProperties;
    extProperties.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extProperties.data());

    bool supportsDescriptorBuffers = false;
    for(const auto& extProperty : extProperties)
    {
        if(strcmp(extProperty.extensionName, VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME) == 0)
        {
            supportsDescriptorBuffers = true;
        }
    }

    if(!supportsDescriptorBuffers)
    {
        vkw::utils::Log::Info(
            testName, "Descriptor buffers not available for this physical device, skipping");
        return true;
    }

    VkPhysicalDeviceDescriptorIndexingFeatures availabeDescriptorIndexingFeatures = {};
    availabeDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    availabeDescriptorIndexingFeatures.pNext = nullptr;

    VkPhysicalDeviceFeatures2 availablePhysicalDeviceFeatures = {};
    availablePhysicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    availablePhysicalDeviceFeatures.pNext = &availabeDescriptorIndexingFeatures;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &availablePhysicalDeviceFeatures);

    const bool supportsIndexing
        = (availabeDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount != VK_FALSE)
          && (availabeDescriptorIndexingFeatures.descriptorBindingPartiallyBound != VK_FALSE);

    std::vector<const char*> deviceExtensions = {VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME};

    VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {};
    descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    descriptorBufferFeatures.pNext = nullptr;
    descriptorBufferFeatures.descriptorBuffer = VK_TRUE;

    VkPhysicalDeviceBufferDeviceAddressFeatures descriptorBufferAddressEnabled = {};
    descriptorBufferAddressEnabled.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    descriptorBufferAddressEnabled.pNext = &descriptorBufferFeatures;
    descriptorBufferAddressEnabled.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingEnabled = availabeDescriptorIndexingFeatures;
    descriptorIndexingEnabled.pNext = &descriptorBufferAddressEnabled;

    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        device.init(instance, physicalDevice, deviceExtensions, {}, &descriptorIndexingEnabled));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    vkw::utils::Log::Info(testName, "Checking storage buffer descriptor buffer...");
    for(size_t i = 1; i <= 16; ++i)
    {
        if(!testStorageBufferDescriptorBuffer(device, i, 1024))
        {
            vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
            failedTests++;
        }
        totalTests++;
    }

    if(supportsIndexing
       && (availabeDescriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind != VK_FALSE))
    {
        vkw::utils::Log::Info(testName, "Checking storage buffer descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageBufferDescriptorBufferIndexing(device, i, 1024))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    if(supportsIndexing
       && (availabeDescriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind != VK_FALSE))
    {
        vkw::utils::Log::Info(testName, "Checking storage image descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageImageDescriptorBufferIndexing(device, i, 256))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    if(supportsIndexing
       && (availabeDescriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind != VK_FALSE))
    {
        vkw::utils::Log::Info(testName, "Checking storage texel buffer descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageTexelBufferDescriptorBufferIndexing(device, i, 1024))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    if(supportsIndexing
       && (availabeDescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind != VK_FALSE))
    {
        vkw::utils::Log::Info(testName, "Checking uniform buffer descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testUniformBufferDescriptorBufferIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    if(supportsIndexing
       && (availabeDescriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind != VK_FALSE))
    {
        vkw::utils::Log::Info(testName, "Checking uniform texel buffer descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testUniformTexelBufferDescriptorBufferIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    if(supportsIndexing
       && (availabeDescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind != VK_FALSE))
    {
        vkw::utils::Log::Info(testName, "Checking sampled image descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testSampledImageDescriptorBufferIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }

        vkw::utils::Log::Info(testName, "Checking combined image sampler descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testCombinedImageSamplerDescriptorBufferIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return failedTests == 0;
}

// -----------------------------------------------------------------------------------------------------------

bool testStorageBufferDescriptorBuffer(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    std::vector<vkw::DeviceBuffer<float>> bufferList;
    bufferList.reserve(descriptorCount);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        bufferList.emplace_back(
            device, bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        VKW_CHECK_BOOL_RETURN_FALSE(bufferList.back().initialized());
    }

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSetLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        float tag;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline fillPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageBufferDescriptorBufferComp),
        sizeof(fillStorageBufferDescriptorBufferComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        fillPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutOffset = descriptorSetLayout.getLayoutBindingOffset(0);
    const auto layoutSize = descriptorSetLayout.getLayoutSize();

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, descriptorCount * layoutSize));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = layoutOffset + i * layoutSize;
        descriptorBuffer.writeStorageBuffer(offset, bufferList[i]);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.bindComputePipeline(fillPipeline);

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const Params params = {static_cast<float>(i)};
        cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, i * layoutSize);
        cmdBuffer.pushConstants(pipelineLayout, params, VK_SHADER_STAGE_COMPUTE_BIT);
        cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));
    }

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto bufferData = std::make_unique<float[]>(bufferSize);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::downloadFromDeviceBuffer(device, bufferList[i], bufferData.get(), bufferSize));
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::checkBufferContents<float>(bufferData.get(), static_cast<float>(i), bufferSize, 1));
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testStorageBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    std::vector<vkw::DeviceBuffer<float>> bufferList;
    bufferList.reserve(descriptorCount);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        bufferList.emplace_back(
            device, bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        VKW_CHECK_BOOL_RETURN_FALSE(bufferList.back().initialized());
    }

    vkw::DescriptorSetLayout setLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.init(device));
    setLayout.addBindings<vkw::DescriptorType::StorageBuffer>(
        VK_SHADER_STAGE_COMPUTE_BIT, 0, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
        uint32_t maxBufferCount;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, setLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline fillBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillBuffersPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageBuffersDescriptorBufferIndexingComp),
        sizeof(fillStorageBuffersDescriptorBufferIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        fillBuffersPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    vkw::ComputePipeline updateBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(updateBuffersPipeline.init(
        device, reinterpret_cast<const char*>(updateStorageBuffersDescriptorBufferIndexingComp),
        sizeof(updateStorageBuffersDescriptorBufferIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        updateBuffersPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutOffset = setLayout.getLayoutBindingOffset(0);
    const auto layoutSize = setLayout.getLayoutSize();
    const auto storageBufferDescSize = device.getDescriptorBufferProperties().storageBufferDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, layoutSize));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = layoutOffset + i * storageBufferDescSize;
        descriptorBuffer.writeStorageBuffer(offset, bufferList[i]);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);

    Params fillParams = {0, static_cast<uint32_t>(descriptorCount), static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(fillBuffersPipeline);
    cmdBuffer.pushConstants(pipelineLayout, fillParams, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));

    cmdBuffer.memoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createMemoryBarrier(
            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const Params updateParams
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(1), static_cast<uint32_t>(descriptorCount)};
        cmdBuffer.bindComputePipeline(updateBuffersPipeline);
        cmdBuffer.pushConstants(pipelineLayout, updateParams, VK_SHADER_STAGE_COMPUTE_BIT);
        cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));
    }

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    return downloadAndCheckBuffers(device, bufferList, bufferSize, 1.0f);
}

bool testStorageImageDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize)
{
    std::vector<vkw::StorageImage> imageList{descriptorCount};
    std::vector<vkw::ImageView> imageViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& image = imageList[i];
        auto& imageView = imageViews[i];
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        VKW_CHECK_BOOL_RETURN_FALSE(image.init(
            device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        VKW_CHECK_BOOL_RETURN_FALSE(
            imageView.init(device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange));

        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));
    }

    vkw::DescriptorSetLayout setLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.init(device));
    setLayout.addBindings<vkw::DescriptorType::StorageImage>(
        VK_SHADER_STAGE_COMPUTE_BIT, 0, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
        uint32_t maxImageCount;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, setLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline fillImagesPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillImagesPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageImagesDescriptorIndexingComp),
        sizeof(fillStorageImagesDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        fillImagesPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    vkw::ComputePipeline updateImagesPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(updateImagesPipeline.init(
        device, reinterpret_cast<const char*>(updateStorageImagesDescriptorIndexingComp),
        sizeof(updateStorageImagesDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        updateImagesPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutOffset = setLayout.getLayoutBindingOffset(0);
    const auto layoutSize = setLayout.getLayoutSize();
    const auto storageImageDescSize = device.getDescriptorBufferProperties().storageImageDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, layoutSize));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = layoutOffset + i * storageImageDescSize;
        descriptorBuffer.writeStorageImage(offset, imageViews[i]);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);

    Params fillParams = {0, static_cast<uint32_t>(descriptorCount), static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(fillImagesPipeline);
    cmdBuffer.pushConstants(pipelineLayout, fillParams, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(
        vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16),
        vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16));

    cmdBuffer.memoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createMemoryBarrier(
            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const Params updateParams
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(1), static_cast<uint32_t>(descriptorCount)};
        cmdBuffer.bindComputePipeline(updateImagesPipeline);
        cmdBuffer.pushConstants(pipelineLayout, updateParams, VK_SHADER_STAGE_COMPUTE_BIT);
        cmdBuffer.dispatch(
            vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16),
            vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16));
    }

    cmdBuffer.end();

    vkw::Fence fence(device);
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto imgData = std::make_unique<float[]>(imgSize * imgSize);
    float index = 1.0f;
    for(const auto& image : imageList)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::downloadImage<float>(
                device, image, imgData.get(), static_cast<uint32_t>(imgSize),
                static_cast<uint32_t>(imgSize)));
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::checkBufferContents<float>(imgData.get(), index, imgSize, imgSize));
        index += 1.0f;
    }

    return true;
}

bool testStorageTexelBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    std::vector<vkw::HostDeviceBuffer<float>> bufferList{descriptorCount};
    std::vector<vkw::BufferView> bufferViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        VKW_CHECK_BOOL_RETURN_FALSE(buffer.init(
            device, bufferSize,
            VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT));

        auto& bufferView = bufferViews[i];
        VKW_CHECK_BOOL_RETURN_FALSE(bufferView.init(device, buffer, VK_FORMAT_R32_SFLOAT));
    }

    vkw::DescriptorSetLayout setLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.init(device));
    setLayout.addBindings<vkw::DescriptorType::StorageTexelBuffer>(
        VK_SHADER_STAGE_COMPUTE_BIT, 0, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
        uint32_t maxBufferCount;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, setLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline fillBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillBuffersPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageTexelBuffersDescriptorIndexingComp),
        sizeof(fillStorageTexelBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        fillBuffersPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    vkw::ComputePipeline updateBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(updateBuffersPipeline.init(
        device, reinterpret_cast<const char*>(updateStorageTexelBuffersDescriptorIndexingComp),
        sizeof(updateStorageTexelBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        updateBuffersPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutOffset = setLayout.getLayoutBindingOffset(0);
    const auto layoutSize = setLayout.getLayoutSize();
    const auto texelBufferDescSize = device.getDescriptorBufferProperties().storageTexelBufferDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, layoutSize));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = layoutOffset + i * texelBufferDescSize;
        descriptorBuffer.writeStorageTexelBuffer(offset, bufferList[i], VK_FORMAT_R32_SFLOAT);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);

    Params fillParams = {0, static_cast<uint32_t>(descriptorCount), static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(fillBuffersPipeline);
    cmdBuffer.pushConstants(pipelineLayout, fillParams, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));

    cmdBuffer.memoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createMemoryBarrier(
            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const Params updateParams
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(1), static_cast<uint32_t>(descriptorCount)};
        cmdBuffer.bindComputePipeline(updateBuffersPipeline);
        cmdBuffer.pushConstants(pipelineLayout, updateParams, VK_SHADER_STAGE_COMPUTE_BIT);
        cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));
    }

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto bufferData = std::make_unique<float[]>(bufferSize);
    float index = 1.0f;
    for(const auto& buffer : bufferList)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(TestUtils::downloadBuffer(device, buffer, bufferData.get(), bufferSize));
        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::checkBufferContents<float>(bufferData.get(), index, bufferSize, 1));
        index += 1.0f;
    }

    return true;
}

bool testUniformBufferDescriptorBufferIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    std::vector<vkw::HostCoherentBuffer<float>> bufferList{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        VKW_CHECK_BOOL_RETURN_FALSE(buffer.init(
            device, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));
        buffer[0] = static_cast<float>(i + 1);
    }

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    vkw::DescriptorSetLayout setLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.init(device));
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    setLayout.addBindings<vkw::DescriptorType::UniformBuffer>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        uint32_t count;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, setLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readUniformBuffersDescriptorIndexingComp),
        sizeof(readUniformBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        readPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutSize = setLayout.getLayoutSize();
    const auto outputOffset = setLayout.getLayoutBindingOffset(0);
    const auto uboOffset = setLayout.getLayoutBindingOffset(1);
    const auto uniformBufferDescSize = device.getDescriptorBufferProperties().uniformBufferDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, layoutSize));

    descriptorBuffer.writeStorageBuffer(outputOffset, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = uboOffset + i * uniformBufferDescSize;
        descriptorBuffer.writeUniformBuffer(offset, bufferList[i]);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.pushConstants(pipelineLayout, params, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadFromDeviceBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1))
        {
            return false;
        }
    }

    return true;
}

bool testUniformTexelBufferDescriptorBufferIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    std::vector<vkw::HostCoherentBuffer<float>> bufferList{descriptorCount};
    std::vector<vkw::BufferView> bufferViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        VKW_CHECK_BOOL_RETURN_FALSE(buffer.init(
            device, 1, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT));
        buffer[0] = static_cast<float>(i + 1);

        auto& bufferView = bufferViews[i];
        VKW_CHECK_BOOL_RETURN_FALSE(bufferView.init(device, buffer, VK_FORMAT_R32_SFLOAT));
    }

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    vkw::DescriptorSetLayout setLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.init(device));
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    setLayout.addBindings<vkw::DescriptorType::UniformTexelBuffer>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        uint32_t count;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, setLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readUniformTexelBuffersDescriptorIndexingComp),
        sizeof(readUniformTexelBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        readPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutSize = setLayout.getLayoutSize();
    const auto outputOffset = setLayout.getLayoutBindingOffset(0);
    const auto texelOffset = setLayout.getLayoutBindingOffset(1);
    const auto uniformTexelBufferDescSize
        = device.getDescriptorBufferProperties().uniformTexelBufferDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, layoutSize));

    descriptorBuffer.writeStorageBuffer(outputOffset, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = texelOffset + i * uniformTexelBufferDescSize;
        descriptorBuffer.writeUniformTexelBuffer(offset, bufferList[i], VK_FORMAT_R32_SFLOAT);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.pushConstants(pipelineLayout, params, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadFromDeviceBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1))
        {
            return false;
        }
    }

    return true;
}

bool testSampledImageDescriptorBufferIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    static constexpr size_t imgSize = 4;

    std::vector<vkw::Texture> imageList{descriptorCount};
    std::vector<vkw::ImageView> imageViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& image = imageList[i];
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        VKW_CHECK_BOOL_RETURN_FALSE(image.init(
            device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        VKW_CHECK_BOOL_RETURN_FALSE(
            imageViews[i].init(device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange));

        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));

        std::vector<float> pixels(imgSize * imgSize, static_cast<float>(i + 1));
        VKW_CHECK_BOOL_RETURN_FALSE(
            (TestUtils::uploadImage<float>(device, pixels.data(), image, imgSize, imgSize)));
    }

    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.maxLod = 0.0f;
    vkw::Sampler sampler{device, samplerCreateInfo};
    VKW_CHECK_BOOL_RETURN_FALSE(sampler.initialized());

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    vkw::DescriptorSetLayout samplerSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(samplerSetLayout.init(device));
    samplerSetLayout.addBinding<vkw::DescriptorType::Sampler>(VK_SHADER_STAGE_ALL, 0);
    VKW_CHECK_BOOL_RETURN_FALSE(
        samplerSetLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    vkw::DescriptorSetLayout resourceSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(resourceSetLayout.init(device));
    resourceSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    resourceSetLayout.addBindings<vkw::DescriptorType::SampledImage>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(
        resourceSetLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    struct Params
    {
        uint32_t count;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(
        device,
        std::vector<std::reference_wrapper<vkw::DescriptorSetLayout>>{samplerSetLayout, resourceSetLayout}));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readSampledImagesDescriptorBufferIndexingComp),
        sizeof(readSampledImagesDescriptorBufferIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        readPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto samplerLayoutSize = samplerSetLayout.getLayoutSize();
    const auto samplerOffset = samplerSetLayout.getLayoutBindingOffset(0);

    vkw::SamplerDescriptorBuffer<vkw::MemoryType::HostCoherent> samplerDescriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(samplerDescriptorBuffer.init(device, samplerLayoutSize));
    samplerDescriptorBuffer.writeSampler(samplerOffset, sampler);

    const auto resourceLayoutSize = resourceSetLayout.getLayoutSize();
    const auto outputOffset = resourceSetLayout.getLayoutBindingOffset(0);
    const auto imageOffset = resourceSetLayout.getLayoutBindingOffset(1);
    const auto sampledImageDescSize = device.getDescriptorBufferProperties().sampledImageDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> resourceDescriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(resourceDescriptorBuffer.init(device, resourceLayoutSize));

    resourceDescriptorBuffer.writeStorageBuffer(outputOffset, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = imageOffset + i * sampledImageDescSize;
        resourceDescriptorBuffer.writeSampledImage(offset, imageViews[i]);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffers({samplerDescriptorBuffer, resourceDescriptorBuffer});
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 1, 1, 0);

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.pushConstants(pipelineLayout, params, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadFromDeviceBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1))
        {
            return false;
        }
    }

    return true;
}

bool testCombinedImageSamplerDescriptorBufferIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    static constexpr size_t imgSize = 4;

    std::vector<vkw::Texture> imageList{descriptorCount};
    std::vector<vkw::ImageView> imageViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& image = imageList[i];
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        VKW_CHECK_BOOL_RETURN_FALSE(image.init(
            device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        VKW_CHECK_BOOL_RETURN_FALSE(
            imageViews[i].init(device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange));

        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));

        std::vector<float> pixels(imgSize * imgSize, static_cast<float>(i + 1));
        VKW_CHECK_BOOL_RETURN_FALSE(
            (TestUtils::uploadImage<float>(device, pixels.data(), image, imgSize, imgSize)));
    }

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    vkw::DescriptorSetLayout setLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.init(device));
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    setLayout.addBindings<vkw::DescriptorType::CombinedImageSampler>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(setLayout.create(VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.maxLod = 0.0f;
    vkw::Sampler sampler{device, samplerCreateInfo};
    VKW_CHECK_BOOL_RETURN_FALSE(sampler.initialized());

    struct Params
    {
        uint32_t count;
    };

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, setLayout));
    pipelineLayout.reservePushConstants<Params>(VK_SHADER_STAGE_COMPUTE_BIT);
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.create());

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readCombinedImageSamplersDescriptorIndexingComp),
        sizeof(readCombinedImageSamplersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(
        readPipeline.createPipeline(pipelineLayout, VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT));

    const auto layoutSize = setLayout.getLayoutSize();
    const auto outputOffset = setLayout.getLayoutBindingOffset(0);
    const auto imageOffset = setLayout.getLayoutBindingOffset(1);
    const auto combinedImageSamplerDescSize
        = device.getDescriptorBufferProperties().combinedImageSamplerDescriptorSize;

    vkw::ResourceDescriptorBuffer<vkw::MemoryType::HostCoherent> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, layoutSize));

    descriptorBuffer.writeStorageBuffer(outputOffset, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        const VkDeviceSize offset = imageOffset + i * combinedImageSamplerDescSize;
        descriptorBuffer.writeCombinedImageSampler(offset, sampler, imageViews[i]);
    }

    auto computeQueue = device.getQueues(vkw::QueueUsageBits::Compute)[0];

    vkw::CommandPool cmdPool{device, computeQueue};
    VKW_CHECK_BOOL_RETURN_FALSE(cmdPool.initialized());

    auto cmdBuffer = cmdPool.createCommandBuffer();
    VKW_CHECK_BOOL_RETURN_FALSE(cmdBuffer.initialized());

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindDescriptorBuffer(descriptorBuffer);
    cmdBuffer.setComputeDescriptorBufferOffsets(pipelineLayout, 0, 0, 0);

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.pushConstants(pipelineLayout, params, VK_SHADER_STAGE_COMPUTE_BIT);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(computeQueue.submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadFromDeviceBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1))
        {
            return false;
        }
    }

    return true;
}
