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

#include "TestDescriptorIndexing.hpp"

#include "Utils.hpp"

#include <memory>
#include <vkw/high_level/Types.hpp>
#include <vkw/vkw.hpp>

static const char* testName = "DescriptorIndexingTest";

// -----------------------------------------------------------------------------------------------------------

static bool testStorageBufferDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testStorageImageDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize);

static bool testStorageTexelBufferDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testUniformBufferDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount);

static bool testUniformTexelBufferDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount);

static bool testSampledImageDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount);

static bool testCombinedImageSamplerDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount);

// -----------------------------------------------------------------------------------------------------------

static const uint32_t fillStorageBuffersDescriptorIndexingComp[] = {
#include "spv/FillStorageBuffersDescriptorIndexing.comp.spv"
};
static const uint32_t updateStorageBuffersDescriptorIndexingComp[] = {
#include "spv/UpdateStorageBuffersDescriptorIndexing.comp.spv"
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

static const uint32_t readSampledImagesDescriptorIndexingComp[] = {
#include "spv/ReadSampledImagesDescriptorIndexing.comp.spv"
};

static const uint32_t readCombinedImageSamplersDescriptorIndexingComp[] = {
#include "spv/ReadCombinedImageSamplersDescriptorIndexing.comp.spv"
};

// -----------------------------------------------------------------------------------------------------------

bool launchDescriptorIndexingTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    const std::vector<const char*> requiredExtensions = {};

    VkPhysicalDeviceDescriptorIndexingFeatures availabeDescriptorIndexingFeatures = {};
    availabeDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    availabeDescriptorIndexingFeatures.pNext = nullptr;

    VkPhysicalDeviceFeatures2 availablePhysicalDeviceFeatures = {};
    availablePhysicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    availablePhysicalDeviceFeatures.pNext = &availabeDescriptorIndexingFeatures;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &availablePhysicalDeviceFeatures);

    if((availabeDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount == VK_FALSE)
       || (availabeDescriptorIndexingFeatures.descriptorBindingPartiallyBound == VK_FALSE))
    {
        vkw::utils::Log::Info(
            testName, "Descriptor indexing not available for this physical device, skipping");
        return true;
    }

    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        device.init(instance, physicalDevice, requiredExtensions, {}, &availabeDescriptorIndexingFeatures));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    // Storage buffer descriptor indexing
    if(availabeDescriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking storage buffer descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageBufferDescriptorIndexing(device, i, 1024))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    // Storage image descriptor indexing
    if(availabeDescriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking storage image descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageImageDescriptorIndexing(device, i, 256))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    // Storage texel buffer descriptor indexing
    if(availabeDescriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking storage texel buffer descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageTexelBufferDescriptorIndexing(device, i, 1024))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    // Uniform buffer descriptor indexing
    if(availabeDescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking uniform buffer descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testUniformBufferDescriptorIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    // Uniform texel buffer descriptor indexing
    if(availabeDescriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking uniform texel buffer descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testUniformTexelBufferDescriptorIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    // Sampled image and combined image sampler descriptor indexing
    if(availabeDescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking sampled image descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testSampledImageDescriptorIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }

        vkw::utils::Log::Info(testName, "Checking combined image sampler descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testCombinedImageSamplerDescriptorIndexing(device, i))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testStorageBufferDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    std::vector<vkw::StorageBuffer<float>> bufferList{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        if(buffer.init(
               device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image");
            return false;
        }
    }

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 1;
    bindingFlagsCreateInfo.pBindingFlags = &bindingFlags;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBindings<vkw::DescriptorType::StorageBuffer>(
        VK_SHADER_STAGE_ALL, 0, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, static_cast<uint32_t>(descriptorCount),
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindStorageBuffer(0, static_cast<uint32_t>(i), bufferList[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
        uint32_t maxBufferCount;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline fillBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillBuffersPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageBuffersDescriptorIndexingComp),
        sizeof(fillStorageBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(fillBuffersPipeline.createPipeline(pipelineLayout));

    vkw::ComputePipeline updateBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(updateBuffersPipeline.init(
        device, reinterpret_cast<const char*>(updateStorageBuffersDescriptorIndexingComp),
        sizeof(updateStorageBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(updateBuffersPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    // Fill buffers
    Params params = {0, static_cast<uint32_t>(descriptorCount), static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(fillBuffersPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));

    // Insert memory barrier
    cmdBuffer.memoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createMemoryBarrier(
            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT));

    // Update buffers
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        Params params
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(1), static_cast<uint32_t>(descriptorCount)};
        cmdBuffer.bindComputePipeline(updateBuffersPipeline);
        cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
        cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
        cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));
    }

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
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

// -----------------------------------------------------------------------------------------------------------

bool testStorageImageDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize)
{
    std::vector<vkw::StorageImage> imageList{descriptorCount};
    std::vector<vkw::ImageView> imageViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& image = imageList[i];
        auto& imageView = imageViews[i];
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        if(image.init(
               device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent,
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image");
            return false;
        }

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        if(imageView.init(device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image view");
            return false;
        }

        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));
    }

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 1;
    bindingFlagsCreateInfo.pBindingFlags = &bindingFlags;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBindings<vkw::DescriptorType::StorageImage>(
        VK_SHADER_STAGE_ALL, 0, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, static_cast<uint32_t>(descriptorCount),
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindStorageImage(0, static_cast<uint32_t>(i), imageViews[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
        uint32_t maxImageCount;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline fillImagesPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillImagesPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageImagesDescriptorIndexingComp),
        sizeof(fillStorageImagesDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(fillImagesPipeline.createPipeline(pipelineLayout));

    vkw::ComputePipeline updateImagesPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(updateImagesPipeline.init(
        device, reinterpret_cast<const char*>(updateStorageImagesDescriptorIndexingComp),
        sizeof(updateStorageImagesDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(updateImagesPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    // Fill images
    Params params = {0, static_cast<uint32_t>(descriptorCount), static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(fillImagesPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(
        vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16),
        vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16));

    // Insert memory barrier
    cmdBuffer.memoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createMemoryBarrier(
            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT));

    // Update images
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        Params params
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(1), static_cast<uint32_t>(descriptorCount)};
        cmdBuffer.bindComputePipeline(updateImagesPipeline);
        cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
        cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
        cmdBuffer.dispatch(
            vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16),
            vkw::utils::divUp(static_cast<uint32_t>(imgSize), 16));
    }

    cmdBuffer.end();

    vkw::Fence fence(device);
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
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

// -----------------------------------------------------------------------------------------------------------

bool testStorageTexelBufferDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    std::vector<vkw::HostDeviceBuffer<float>> bufferList{descriptorCount};
    std::vector<vkw::BufferView> bufferViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        if(buffer.init(
               device, bufferSize,
               VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                   | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing buffer");
            return false;
        }

        auto& bufferView = bufferViews[i];
        if(bufferView.init(device, buffer, VK_FORMAT_R32_SFLOAT) == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing buffer view");
            return false;
        }
    }

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 1;
    bindingFlagsCreateInfo.pBindingFlags = &bindingFlags;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBindings<vkw::DescriptorType::StorageTexelBuffer>(
        VK_SHADER_STAGE_ALL, 0, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, static_cast<uint32_t>(descriptorCount),
        {VkDescriptorPoolSize{
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindStorageTexelBuffer(0, static_cast<uint32_t>(i), bufferViews[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
        uint32_t maxBufferCount;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline fillBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(fillBuffersPipeline.init(
        device, reinterpret_cast<const char*>(fillStorageTexelBuffersDescriptorIndexingComp),
        sizeof(fillStorageTexelBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(fillBuffersPipeline.createPipeline(pipelineLayout));

    vkw::ComputePipeline updateBuffersPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(updateBuffersPipeline.init(
        device, reinterpret_cast<const char*>(updateStorageTexelBuffersDescriptorIndexingComp),
        sizeof(updateStorageTexelBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(updateBuffersPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    Params params = {0, static_cast<uint32_t>(descriptorCount), static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(fillBuffersPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));

    cmdBuffer.memoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createMemoryBarrier(
            VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        Params params
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(1), static_cast<uint32_t>(descriptorCount)};
        cmdBuffer.bindComputePipeline(updateBuffersPipeline);
        cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
        cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
        cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(bufferSize), 256));
    }

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
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

// -----------------------------------------------------------------------------------------------------------

bool testUniformBufferDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    std::vector<vkw::HostStagingBuffer<float>> bufferList{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        if(buffer.init(device, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing buffer");
            return false;
        }
        buffer[0] = static_cast<float>(i + 1);
    }

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    const VkDescriptorBindingFlags bindingFlagsList[2] = {0, bindingFlags};
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 2;
    bindingFlagsCreateInfo.pBindingFlags = bindingFlagsList;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    descriptorSetLayout.addBindings<vkw::DescriptorType::UniformBuffer>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, 1,
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
         VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    descriptorSet.bindStorageBuffer(0, 0, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindUniformBuffer(1, static_cast<uint32_t>(i), bufferList[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t count;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readUniformBuffersDescriptorIndexingComp),
        sizeof(readUniformBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1)) { return false; }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testUniformTexelBufferDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    std::vector<vkw::HostStagingBuffer<float>> bufferList{descriptorCount};
    std::vector<vkw::BufferView> bufferViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& buffer = bufferList[i];
        if(buffer.init(device, 1, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing buffer");
            return false;
        }
        buffer[0] = static_cast<float>(i + 1);

        auto& bufferView = bufferViews[i];
        if(bufferView.init(device, buffer, VK_FORMAT_R32_SFLOAT) == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing buffer view");
            return false;
        }
    }

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    const VkDescriptorBindingFlags bindingFlagsList[2] = {0, bindingFlags};
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 2;
    bindingFlagsCreateInfo.pBindingFlags = bindingFlagsList;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    descriptorSetLayout.addBindings<vkw::DescriptorType::UniformTexelBuffer>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, 1,
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
         VkDescriptorPoolSize{
             VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    descriptorSet.bindStorageBuffer(0, 0, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindUniformTexelBuffer(1, static_cast<uint32_t>(i), bufferViews[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t count;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readUniformTexelBuffersDescriptorIndexingComp),
        sizeof(readUniformTexelBuffersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1)) { return false; }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testSampledImageDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    static constexpr size_t imgSize = 4;

    std::vector<vkw::Texture> imageList{descriptorCount};
    std::vector<vkw::ImageView> imageViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& image = imageList[i];
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        if(image.init(
               device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent,
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image");
            return false;
        }

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        if(imageViews[i].init(device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image view");
            return false;
        }

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
        device, descriptorCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    const VkDescriptorBindingFlags bindingFlagsList[3] = {0, 0, bindingFlags};
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 3;
    bindingFlagsCreateInfo.pBindingFlags = bindingFlagsList;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBinding<vkw::DescriptorType::Sampler>(VK_SHADER_STAGE_ALL, 0);
    descriptorSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 1);
    descriptorSetLayout.addBindings<vkw::DescriptorType::SampledImage>(
        VK_SHADER_STAGE_ALL, 2, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, 1,
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLER, 1},
         VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
         VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    descriptorSet.bindSampler(0, 0, sampler);
    descriptorSet.bindStorageBuffer(1, 0, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindSampledImage(2, static_cast<uint32_t>(i), imageViews[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t count;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readSampledImagesDescriptorIndexingComp),
        sizeof(readSampledImagesDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1)) { return false; }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testCombinedImageSamplerDescriptorIndexing(const vkw::Device& device, const size_t descriptorCount)
{
    static constexpr size_t imgSize = 4;

    std::vector<vkw::Texture> imageList{descriptorCount};
    std::vector<vkw::ImageView> imageViews{descriptorCount};
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        auto& image = imageList[i];
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        if(image.init(
               device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent,
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image");
            return false;
        }

        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        if(imageViews[i].init(device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange)
           == false)
        {
            vkw::utils::Log::Error(testName, "Error initializing image view");
            return false;
        }

        VKW_CHECK_BOOL_RETURN_FALSE(
            TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));

        std::vector<float> pixels(imgSize * imgSize, static_cast<float>(i + 1));
        VKW_CHECK_BOOL_RETURN_FALSE(
            (TestUtils::uploadImage<float>(device, pixels.data(), image, imgSize, imgSize)));
    }

    vkw::HostDeviceBuffer<float> outputBuffer{
        device, descriptorCount, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VKW_CHECK_BOOL_RETURN_FALSE(outputBuffer.initialized());

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
                                                  | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
                                                  | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
    const VkDescriptorBindingFlags bindingFlagsList[2] = {0, bindingFlags};
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 2;
    bindingFlagsCreateInfo.pBindingFlags = bindingFlagsList;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.init(device));
    descriptorSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_ALL, 0);
    descriptorSetLayout.addBindings<vkw::DescriptorType::CombinedImageSampler>(
        VK_SHADER_STAGE_ALL, 1, static_cast<uint32_t>(descriptorCount));
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

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

    vkw::DescriptorPool descriptorPool{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool.init(
        device, 1,
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
         VkDescriptorPoolSize{
             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));
    descriptorSet.bindStorageBuffer(0, 0, outputBuffer);
    for(size_t i = 0; i < descriptorCount; ++i)
    {
        descriptorSet.bindCombinedImageSampler(1, static_cast<uint32_t>(i), sampler, imageViews[i]);
    }

    vkw::PipelineLayout pipelineLayout{};
    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t count;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline readPipeline{};
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.init(
        device, reinterpret_cast<const char*>(readCombinedImageSamplersDescriptorIndexingComp),
        sizeof(readCombinedImageSamplersDescriptorIndexingComp)));
    VKW_CHECK_BOOL_RETURN_FALSE(readPipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();

    Params params = {static_cast<uint32_t>(descriptorCount)};
    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(descriptorCount), 64));

    cmdBuffer.end();

    vkw::Fence fence{device};
    VKW_CHECK_BOOL_RETURN_FALSE(fence.initialized());
    VKW_CHECK_VK_RETURN_FALSE(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence));
    VKW_CHECK_BOOL_RETURN_FALSE(fence.wait());

    auto resultData = std::make_unique<float[]>(descriptorCount);
    VKW_CHECK_BOOL_RETURN_FALSE(
        TestUtils::downloadBuffer(device, outputBuffer, resultData.get(), descriptorCount));

    for(size_t i = 0; i < descriptorCount; ++i)
    {
        if(resultData[i] != static_cast<float>(i + 1)) { return false; }
    }

    return true;
}
