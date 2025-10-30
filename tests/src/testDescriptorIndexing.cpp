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

#include "Utils.hpp"

#include <memory>
#include <vkw/high_level/Types.hpp>
#include <vkw/vkw.hpp>

static const char* logTag = "DescriptorIndexingTest";

static bool testStorageBufferDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testStorageImageDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize);

static bool checkBufferContent(const float* data, const float val, const size_t w, const size_t h);

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

// -----------------------------------------------------------------------------------------------------------

bool launchDescriptorIndexingTestsTest()
{
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    vkw::Instance instance{};
    VKW_CHECK_BOOL_RETURN_FALSE(instance.init(instanceLayers, {}));

    const std::vector<const char*> requiredExtensions = {VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME};

    VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {};
    descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    descriptorBufferFeatures.pNext = nullptr;
    descriptorBufferFeatures.descriptorBuffer = VK_TRUE;
    descriptorBufferFeatures.descriptorBufferPushDescriptors = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
    descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptorIndexingFeatures.pNext = &descriptorBufferFeatures;
    descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
    descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
    bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceAddressFeatures.pNext = &descriptorIndexingFeatures;
    bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

    const auto supportedDevices = vkw::Device::listSupportedDevices(
        instance, requiredExtensions, {}, bufferDeviceAddressFeatures, descriptorIndexingFeatures,
        descriptorBufferFeatures);

    if(supportedDevices.empty())
    {
        vkw::utils::Log::Warning(logTag, "No supported device found, skipping test");
        return EXIT_SUCCESS;
    }

    for(const auto physicalDevice : supportedDevices)
    {
        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        if((properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
           && (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU))
        {
            continue;
        }

        vkw::Device device;
        VKW_CHECK_BOOL_RETURN_FALSE(
            device.init(instance, physicalDevice, requiredExtensions, {}, &bufferDeviceAddressFeatures));

        vkw::utils::Log::Info(logTag, "Testing device: %s", device.getProperties().deviceName);

        // Storage buffer descriptor indexing
        vkw::utils::Log::Info(logTag, "Checking storage buffer descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            const bool res = testStorageBufferDescriptorIndexing(device, i, 1024);
            vkw::utils::Log::Info(logTag, "  Descriptor count %zu - %s", i, (res == true) ? "OK" : "FAILED");
        }

        // Storage image descriptor indexing
        vkw::utils::Log::Info(logTag, "Checking storage image descriptor indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            const bool res = testStorageImageDescriptorIndexing(device, i, 256);
            vkw::utils::Log::Info(logTag, "  Descriptor count %zu - %s", i, (res == true) ? "OK" : "FAILED");
        }
    }

    return EXIT_SUCCESS;
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
            vkw::utils::Log::Error(logTag, "Error initializing image");
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
        descriptorSet.bindStorageBufferIndex(0, bufferList[i], static_cast<uint32_t>(i));
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
    cmdBuffer.bindComputeDescriptorSets(pipelineLayout, 0, {descriptorSet});
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
        VKW_CHECK_BOOL_RETURN_FALSE(downloadBuffer(device, buffer, bufferData.get(), bufferSize));
        VKW_CHECK_BOOL_RETURN_FALSE(checkBufferContent(bufferData.get(), index, bufferSize, 1));
        index += 1.0f;
    }

    return true;
}

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
            vkw::utils::Log::Error(logTag, "Error initializing image");
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
            vkw::utils::Log::Error(logTag, "Error initializing image view");
            return false;
        }

        VKW_CHECK_BOOL_RETURN_FALSE(
            changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL));
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
        descriptorSet.bindStorageImageIndex(0, imageViews[i], static_cast<uint32_t>(i));
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
            downloadImage<float>(
                device, image, imgData.get(), static_cast<uint32_t>(imgSize),
                static_cast<uint32_t>(imgSize)));
        VKW_CHECK_BOOL_RETURN_FALSE(checkBufferContent(imgData.get(), index, imgSize, imgSize));
        index += 1.0f;
    }

    return true;
}

bool checkBufferContent(const float* data, const float val, const size_t w, const size_t h)
{
    const size_t res = w * h;
    for(size_t i = 0; i < res; ++i)
    {
        if(data[i] != val) { return false; }
    }
    return true;
}