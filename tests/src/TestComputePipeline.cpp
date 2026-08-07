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

#include "TestComputePipeline.hpp"

#include "Utils.hpp"

#include <vector>
#include <vkw/vkw.hpp>

static const char* testName = "ComputePipelineTest";

// -----------------------------------------------------------------------------------------------------------

static bool testComputePipelineMove(const vkw::Device& device);
static bool testPipelineLayoutMove(const vkw::Device& device);
static bool testBufferToImageFormatReinterpret(const vkw::Device& device);
static bool testBufferToImageSwizzle(const vkw::Device& device);
static bool testComputePipelineVectorAdd(const vkw::Device& device);
static bool testComputePipelineUniformScale(const vkw::Device& device);
static bool testComputePipelineTexelBuffer(const vkw::Device& device);
static bool testPushConstantOffset(const vkw::Device& device);
static bool testSpecializationConstantsDefault(const vkw::Device& device);
static bool testSpecializationConstantsCustom(const vkw::Device& device);
static bool testComputePipelineMultiSetLayout(const vkw::Device& device);

// -----------------------------------------------------------------------------------------------------------

static const uint32_t writeImageR32Comp[] = {
#include "spv/WriteImageR32.comp.spv"
};
static const uint32_t readImageRGBA8ToBufferComp[] = {
#include "spv/ReadImageRGBA8ToBuffer.comp.spv"
};
static const uint32_t readImageRGBA8SamplerToBufferComp[] = {
#include "spv/ReadImageRGBA8SamplerToBuffer.comp.spv"
};
static const uint32_t vectorAddComp[] = {
#include "spv/VectorAdd.comp.spv"
};
static const uint32_t uniformScaleComp[] = {
#include "spv/UniformScale.comp.spv"
};
static const uint32_t texelBufferCopyComp[] = {
#include "spv/TexelBufferCopy.comp.spv"
};
static const uint32_t pushConstantOffsetComp[] = {
#include "spv/PushConstantOffset.comp.spv"
};
static const uint32_t specializationConstantSingleComp[] = {
#include "spv/SpecializationConstantSingle.comp.spv"
};
static const uint32_t multiSetLayoutComp[] = {
#include "spv/MultiSetLayout.comp.spv"
};

// -----------------------------------------------------------------------------------------------------------

bool launchComputePipelineTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(device.init(instance, physicalDevice, {}, {}));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    vkw::utils::Log::Info(testName, "Checking ComputePipeline move semantics...");
    if(!testComputePipelineMove(device))
    {
        vkw::utils::Log::Warning(testName, "  Move semantics - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking PipelineLayout move semantics...");
    if(!testPipelineLayoutMove(device))
    {
        vkw::utils::Log::Warning(testName, "  Move semantics - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking buffer to image format reinterpretation...");
    if(!testBufferToImageFormatReinterpret(device))
    {
        vkw::utils::Log::Warning(testName, "  Format reinterpretation - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking buffer to image with swizzled view...");
    if(!testBufferToImageSwizzle(device))
    {
        vkw::utils::Log::Warning(testName, "  Swizzle - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking vector add compute pipeline...");
    if(!testComputePipelineVectorAdd(device))
    {
        vkw::utils::Log::Warning(testName, "  Vector add - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking uniform scale compute pipeline...");
    if(!testComputePipelineUniformScale(device))
    {
        vkw::utils::Log::Warning(testName, "  Uniform scale - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking texel buffer compute pipeline...");
    if(!testComputePipelineTexelBuffer(device))
    {
        vkw::utils::Log::Warning(testName, "  Texel buffer - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking push constant offset handling...");
    if(!testPushConstantOffset(device))
    {
        vkw::utils::Log::Warning(testName, "  Push constant offset - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking specialization constants (default values)...");
    if(!testSpecializationConstantsDefault(device))
    {
        vkw::utils::Log::Warning(testName, "  Specialization constants (default) - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking specialization constants (custom values)...");
    if(!testSpecializationConstantsCustom(device))
    {
        vkw::utils::Log::Warning(testName, "  Specialization constants (custom) - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking multi-set pipeline layout...");
    if(!testComputePipelineMultiSetLayout(device))
    {
        vkw::utils::Log::Warning(testName, "  Multi-set layout - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return failedTests == 0;
}

// -----------------------------------------------------------------------------------------------------------

bool testComputePipelineMove(const vkw::Device& device)
{
    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 2);
    if(!setLayout.create()) { return false; }

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(device, setLayout)) { return false; }
    if(!pipelineLayout.create()) { return false; }

    vkw::ComputePipeline pipelineA{};
    if(!pipelineA.init(device, reinterpret_cast<const char*>(vectorAddComp), sizeof(vectorAddComp)))
    {
        vkw::utils::Log::Error(testName, "  ComputePipeline init failed");
        return false;
    }
    if(!pipelineA.createPipeline(pipelineLayout))
    {
        vkw::utils::Log::Error(testName, "  ComputePipeline createPipeline failed");
        return false;
    }

    const auto handle = pipelineA.getHandle();

    vkw::ComputePipeline pipelineB{std::move(pipelineA)};
    if(!pipelineB.initialized() || (pipelineB.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  Move-construct failed");
        return false;
    }
    if(pipelineA.initialized() || (pipelineA.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  Moved-from object not cleared");
        return false;
    }

    vkw::ComputePipeline pipelineC{};
    pipelineC = std::move(pipelineB);
    if(!pipelineC.initialized() || (pipelineC.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  Move-assign failed");
        return false;
    }
    if(pipelineB.initialized() || (pipelineB.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  Moved-from object not cleared after assign");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testPipelineLayoutMove(const vkw::Device& device)
{
    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    if(!setLayout.create()) { return false; }

    vkw::PipelineLayout layoutA{};
    if(!layoutA.init(device, setLayout)) { return false; }
    if(!layoutA.create()) { return false; }

    const auto handle = layoutA.getHandle();

    vkw::PipelineLayout layoutB{std::move(layoutA)};
    if(!layoutB.initialized() || (layoutB.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  Move-construct failed");
        return false;
    }
    if(layoutA.initialized() || (layoutA.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  Moved-from object not cleared");
        return false;
    }

    vkw::PipelineLayout layoutC{};
    layoutC = std::move(layoutB);
    if(!layoutC.initialized() || (layoutC.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  Move-assign failed");
        return false;
    }
    if(layoutB.initialized() || (layoutB.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  Moved-from object not cleared after assign");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testBufferToImageFormatReinterpret(const vkw::Device& device)
{
    static constexpr uint32_t w = 8;
    static constexpr uint32_t h = 8;
    static constexpr size_t count = w * h;

    std::vector<uint32_t> pattern(count);
    TestUtils::fillPattern<uint32_t>(pattern.data(), count, 1u);

    vkw::HostToDeviceBuffer<uint32_t> inputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!inputBuffer.initialized()) { return false; }
    if(!inputBuffer.copyFromHost(pattern.data(), count)) { return false; }

    vkw::DeviceToHostBuffer<uint32_t> outputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!outputBuffer.initialized()) { return false; }

    const VkExtent3D extent{w, h, 1};
    vkw::DeviceImage<VK_IMAGE_USAGE_STORAGE_BIT> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, extent, VK_IMAGE_USAGE_STORAGE_BIT};
    if(!image.initialized()) { return false; }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;

    vkw::ImageView viewR32{device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, subresourceRange};
    if(!viewR32.initialized()) { return false; }

    vkw::ImageView viewRGBA8{device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UINT, subresourceRange};
    if(!viewRGBA8.initialized()) { return false; }

    if(!TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        return false;
    }

    vkw::DescriptorSetLayout writeSetLayout{};
    if(!writeSetLayout.init(device)) { return false; }
    writeSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    writeSetLayout.addBinding<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!writeSetLayout.create()) { return false; }

    vkw::DescriptorSetLayout readSetLayout{};
    if(!readSetLayout.init(device)) { return false; }
    readSetLayout.addBinding<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    readSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!readSetLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(
           device, 2,
           {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2}}))
    {
        return false;
    }

    vkw::DescriptorSet writeSet{};
    if(!writeSet.init(device, writeSetLayout, descriptorPool)) { return false; }
    writeSet.bindStorageBuffer(0, 0, inputBuffer);
    writeSet.bindStorageImage(1, 0, viewR32);

    vkw::DescriptorSet readSet{};
    if(!readSet.init(device, readSetLayout, descriptorPool)) { return false; }
    readSet.bindStorageImage(0, 0, viewRGBA8);
    readSet.bindStorageBuffer(1, 0, outputBuffer);

    struct Params
    {
        uint32_t width;
    };

    vkw::PipelineLayout writePipelineLayout{};
    if(!writePipelineLayout.init(device, writeSetLayout)) { return false; }
    writePipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    if(!writePipelineLayout.create()) { return false; }

    vkw::PipelineLayout readPipelineLayout{};
    if(!readPipelineLayout.init(device, readSetLayout)) { return false; }
    readPipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    if(!readPipelineLayout.create()) { return false; }

    vkw::ComputePipeline writePipeline{};
    if(!writePipeline.init(
           device, reinterpret_cast<const char*>(writeImageR32Comp), sizeof(writeImageR32Comp)))
    {
        return false;
    }
    if(!writePipeline.createPipeline(writePipelineLayout)) { return false; }

    vkw::ComputePipeline readPipeline{};
    if(!readPipeline.init(
           device, reinterpret_cast<const char*>(readImageRGBA8ToBufferComp),
           sizeof(readImageRGBA8ToBufferComp)))
    {
        return false;
    }
    if(!readPipeline.createPipeline(readPipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    Params params = {w};

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(writePipeline);
    cmdBuffer.bindComputeDescriptorSet(writePipelineLayout, 0, writeSet);
    cmdBuffer.pushConstants(writePipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(w / 8, h / 8);

    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createImageMemoryBarrier(
            image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL));

    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.bindComputeDescriptorSet(readPipelineLayout, 0, readSet);
    cmdBuffer.pushConstants(readPipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(w / 8, h / 8);
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<uint32_t> result(count);
    if(!outputBuffer.copyToHost(result.data(), count)) { return false; }

    return TestUtils::compareData(pattern.data(), result.data(), count);
}

// -----------------------------------------------------------------------------------------------------------

bool testBufferToImageSwizzle(const vkw::Device& device)
{
    static constexpr uint32_t w = 8;
    static constexpr uint32_t h = 8;
    static constexpr size_t count = w * h;

    std::vector<uint32_t> pattern(count);
    TestUtils::fillPattern<uint32_t>(pattern.data(), count, 1u);

    vkw::HostToDeviceBuffer<uint32_t> inputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!inputBuffer.initialized()) { return false; }
    if(!inputBuffer.copyFromHost(pattern.data(), count)) { return false; }

    vkw::DeviceToHostBuffer<uint32_t> outputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!outputBuffer.initialized()) { return false; }

    const VkExtent3D extent{w, h, 1};
    vkw::DeviceImage<VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_UINT, extent,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT};
    if(!image.initialized()) { return false; }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;

    vkw::ImageView viewR32{device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_UINT, subresourceRange};
    if(!viewR32.initialized()) { return false; }

    VkImageViewCreateInfo swizzledCreateInfo = {};
    swizzledCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    swizzledCreateInfo.image = image.getHandle();
    swizzledCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    swizzledCreateInfo.format = VK_FORMAT_R8G8B8A8_UINT;
    swizzledCreateInfo.components.r = VK_COMPONENT_SWIZZLE_B;
    swizzledCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    swizzledCreateInfo.components.b = VK_COMPONENT_SWIZZLE_R;
    swizzledCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    swizzledCreateInfo.subresourceRange = subresourceRange;

    vkw::ImageView viewRGBA8Swizzled{};
    if(!viewRGBA8Swizzled.init(device, swizzledCreateInfo)) { return false; }

    if(!TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        return false;
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
    if(!sampler.initialized()) { return false; }

    vkw::DescriptorSetLayout writeSetLayout{};
    if(!writeSetLayout.init(device)) { return false; }
    writeSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    writeSetLayout.addBinding<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!writeSetLayout.create()) { return false; }

    vkw::DescriptorSetLayout readSetLayout{};
    if(!readSetLayout.init(device)) { return false; }
    readSetLayout.addBinding<vkw::DescriptorType::CombinedImageSampler>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    readSetLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!readSetLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(
           device, 2,
           {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}}))
    {
        return false;
    }

    vkw::DescriptorSet writeSet{};
    if(!writeSet.init(device, writeSetLayout, descriptorPool)) { return false; }
    writeSet.bindStorageBuffer(0, 0, inputBuffer);
    writeSet.bindStorageImage(1, 0, viewR32);

    vkw::DescriptorSet readSet{};
    if(!readSet.init(device, readSetLayout, descriptorPool)) { return false; }
    readSet.bindCombinedImageSampler(0, 0, sampler, viewRGBA8Swizzled);
    readSet.bindStorageBuffer(1, 0, outputBuffer);

    struct Params
    {
        uint32_t width;
    };

    vkw::PipelineLayout writePipelineLayout{};
    if(!writePipelineLayout.init(device, writeSetLayout)) { return false; }
    writePipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    if(!writePipelineLayout.create()) { return false; }

    vkw::PipelineLayout readPipelineLayout{};
    if(!readPipelineLayout.init(device, readSetLayout)) { return false; }
    readPipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    if(!readPipelineLayout.create()) { return false; }

    vkw::ComputePipeline writePipeline{};
    if(!writePipeline.init(
           device, reinterpret_cast<const char*>(writeImageR32Comp), sizeof(writeImageR32Comp)))
    {
        return false;
    }
    if(!writePipeline.createPipeline(writePipelineLayout)) { return false; }

    vkw::ComputePipeline readPipeline{};
    if(!readPipeline.init(
           device, reinterpret_cast<const char*>(readImageRGBA8SamplerToBufferComp),
           sizeof(readImageRGBA8SamplerToBufferComp)))
    {
        return false;
    }
    if(!readPipeline.createPipeline(readPipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    Params params = {w};

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(writePipeline);
    cmdBuffer.bindComputeDescriptorSet(writePipelineLayout, 0, writeSet);
    cmdBuffer.pushConstants(writePipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(w / 8, h / 8);

    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createImageMemoryBarrier(
            image, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL));

    cmdBuffer.bindComputePipeline(readPipeline);
    cmdBuffer.bindComputeDescriptorSet(readPipelineLayout, 0, readSet);
    cmdBuffer.pushConstants(readPipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(w / 8, h / 8);
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<uint32_t> result(count);
    if(!outputBuffer.copyToHost(result.data(), count)) { return false; }

    for(size_t i = 0; i < count; ++i)
    {
        const uint32_t original = pattern[i];
        const uint32_t r = original & 0xFFu;
        const uint32_t g = (original >> 8) & 0xFFu;
        const uint32_t b = (original >> 16) & 0xFFu;
        const uint32_t a = (original >> 24) & 0xFFu;
        const uint32_t expected = b | (g << 8) | (r << 16) | (a << 24);
        if(result[i] != expected)
        {
            vkw::utils::Log::Error(testName, "  Swizzle mismatch at index %zu", i);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testComputePipelineVectorAdd(const vkw::Device& device)
{
    static constexpr size_t count = 4096;

    std::vector<float> a(count);
    std::vector<float> b(count);
    TestUtils::fillPattern<float>(a.data(), count, 1.0f);
    TestUtils::fillPattern<float>(b.data(), count, 2.0f);

    vkw::HostToDeviceBuffer<float> bufferA{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    vkw::HostToDeviceBuffer<float> bufferB{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    vkw::DeviceToHostBuffer<float> bufferOut{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!bufferA.initialized() || !bufferB.initialized() || !bufferOut.initialized()) { return false; }
    if(!bufferA.copyFromHost(a.data(), count) || !bufferB.copyFromHost(b.data(), count)) { return false; }

    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 2);
    if(!setLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(device, 1, {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3}}))
    {
        return false;
    }

    vkw::DescriptorSet descriptorSet{};
    if(!descriptorSet.init(device, setLayout, descriptorPool)) { return false; }
    descriptorSet.bindStorageBuffer(0, 0, bufferA);
    descriptorSet.bindStorageBuffer(1, 0, bufferB);
    descriptorSet.bindStorageBuffer(2, 0, bufferOut);

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(device, setLayout)) { return false; }
    if(!pipelineLayout.create()) { return false; }

    vkw::ComputePipeline pipeline{};
    if(!pipeline.init(device, reinterpret_cast<const char*>(vectorAddComp), sizeof(vectorAddComp)))
    {
        return false;
    }
    if(!pipeline.createPipeline(pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(count), 256));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<float> result(count);
    if(!bufferOut.copyToHost(result.data(), count)) { return false; }

    for(size_t i = 0; i < count; ++i)
    {
        if(result[i] != a[i] + b[i]) { return false; }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testComputePipelineUniformScale(const vkw::Device& device)
{
    static constexpr size_t count = 4096;
    static constexpr float scaleValue = 3.5f;

    std::vector<float> input(count);
    TestUtils::fillPattern<float>(input.data(), count, 1.0f);

    vkw::HostToDeviceBuffer<float> inputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    vkw::HostStagingBuffer<float> scaleBuffer{device, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
    vkw::DeviceToHostBuffer<float> outputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!inputBuffer.initialized() || !scaleBuffer.initialized() || !outputBuffer.initialized())
    {
        return false;
    }
    if(!inputBuffer.copyFromHost(input.data(), count)) { return false; }
    scaleBuffer[0] = scaleValue;

    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout.addBinding<vkw::DescriptorType::UniformBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 2);
    if(!setLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(
           device, 1,
           {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}}))
    {
        return false;
    }

    vkw::DescriptorSet descriptorSet{};
    if(!descriptorSet.init(device, setLayout, descriptorPool)) { return false; }
    descriptorSet.bindStorageBuffer(0, 0, inputBuffer);
    descriptorSet.bindUniformBuffer(1, 0, scaleBuffer);
    descriptorSet.bindStorageBuffer(2, 0, outputBuffer);

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(device, setLayout)) { return false; }
    if(!pipelineLayout.create()) { return false; }

    vkw::ComputePipeline pipeline{};
    if(!pipeline.init(device, reinterpret_cast<const char*>(uniformScaleComp), sizeof(uniformScaleComp)))
    {
        return false;
    }
    if(!pipeline.createPipeline(pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(count), 256));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<float> result(count);
    if(!outputBuffer.copyToHost(result.data(), count)) { return false; }

    for(size_t i = 0; i < count; ++i)
    {
        if(result[i] != input[i] * scaleValue) { return false; }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testComputePipelineTexelBuffer(const vkw::Device& device)
{
    static constexpr size_t count = 1024;

    std::vector<float> input(count);
    TestUtils::fillPattern<float>(input.data(), count, 1.0f);

    vkw::HostToDeviceBuffer<float> texelBuffer{device, count, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT};
    if(!texelBuffer.initialized()) { return false; }
    if(!texelBuffer.copyFromHost(input.data(), count)) { return false; }

    vkw::BufferView bufferView{};
    if(!bufferView.init(device, texelBuffer, VK_FORMAT_R32_SFLOAT)) { return false; }

    vkw::DeviceToHostBuffer<float> outputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!outputBuffer.initialized()) { return false; }

    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageTexelBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!setLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(
           device, 1,
           {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}}))
    {
        return false;
    }

    vkw::DescriptorSet descriptorSet{};
    if(!descriptorSet.init(device, setLayout, descriptorPool)) { return false; }
    descriptorSet.bindStorageTexelBuffer(0, 0, bufferView);
    descriptorSet.bindStorageBuffer(1, 0, outputBuffer);

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(device, setLayout)) { return false; }
    if(!pipelineLayout.create()) { return false; }

    vkw::ComputePipeline pipeline{};
    if(!pipeline.init(
           device, reinterpret_cast<const char*>(texelBufferCopyComp), sizeof(texelBufferCopyComp)))
    {
        return false;
    }
    if(!pipeline.createPipeline(pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(count), 256));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<float> result(count);
    if(!outputBuffer.copyToHost(result.data(), count)) { return false; }

    return TestUtils::compareData(input.data(), result.data(), count);
}

// -----------------------------------------------------------------------------------------------------------

bool testPushConstantOffset(const vkw::Device& device)
{
    static constexpr size_t count = 256;
    static constexpr uint32_t testOffset = 8;
    static constexpr float testValue = 42.0f;

    std::vector<float> input(count + testOffset);
    TestUtils::fillPattern<float>(input.data(), count + testOffset, 1.0f);

    vkw::HostToDeviceBuffer<float> inputBuffer{
        device, count + testOffset, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    vkw::DeviceToHostBuffer<float> outputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!inputBuffer.initialized() || !outputBuffer.initialized()) { return false; }
    if(!inputBuffer.copyFromHost(input.data(), count + testOffset)) { return false; }

    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!setLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(device, 1, {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}}))
    {
        return false;
    }

    vkw::DescriptorSet descriptorSet{};
    if(!descriptorSet.init(device, setLayout, descriptorPool)) { return false; }
    descriptorSet.bindStorageBuffer(0, 0, inputBuffer);
    descriptorSet.bindStorageBuffer(1, 0, outputBuffer);

    struct Params
    {
        uint32_t unused;
        uint32_t offset;
        float value;
    };

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(device, setLayout)) { return false; }
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    if(!pipelineLayout.create()) { return false; }

    vkw::ComputePipeline pipeline{};
    if(!pipeline.init(
           device, reinterpret_cast<const char*>(pushConstantOffsetComp), sizeof(pushConstantOffsetComp)))
    {
        return false;
    }
    if(!pipeline.createPipeline(pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    Params params = {0, testOffset, testValue};

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(count), 256));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<float> result(count);
    if(!outputBuffer.copyToHost(result.data(), count)) { return false; }

    for(size_t i = 0; i < count; ++i)
    {
        const float expected = input[i + testOffset] + testValue;
        if(result[i] != expected)
        {
            vkw::utils::Log::Error(testName, "  Push constant offset mismatch at index %zu", i);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

static bool runSpecializationConstantsTest(
    const vkw::Device& device, const uint32_t mult, const float bias, const int32_t sign, const bool enable)
{
    static constexpr size_t count = 1024;

    std::vector<float> input(count);
    TestUtils::fillPattern<float>(input.data(), count, 1.0f);

    vkw::HostToDeviceBuffer<float> inputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    vkw::DeviceToHostBuffer<float> outputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!inputBuffer.initialized() || !outputBuffer.initialized()) { return false; }
    if(!inputBuffer.copyFromHost(input.data(), count)) { return false; }

    vkw::DescriptorSetLayout setLayout{};
    if(!setLayout.init(device)) { return false; }
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!setLayout.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(device, 1, {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}}))
    {
        return false;
    }

    vkw::DescriptorSet descriptorSet{};
    if(!descriptorSet.init(device, setLayout, descriptorPool)) { return false; }
    descriptorSet.bindStorageBuffer(0, 0, inputBuffer);
    descriptorSet.bindStorageBuffer(1, 0, outputBuffer);

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(device, setLayout)) { return false; }
    if(!pipelineLayout.create()) { return false; }

    vkw::ComputePipeline pipeline{};
    if(!pipeline.init(
           device, reinterpret_cast<const char*>(specializationConstantSingleComp),
           sizeof(specializationConstantSingleComp)))
    {
        return false;
    }
    pipeline.addSpec<uint32_t>(mult).addSpec<float>(bias).addSpec<int32_t>(sign).addSpec<uint32_t>(
        enable ? 1u : 0u);
    if(!pipeline.createPipeline(pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet);
    cmdBuffer.dispatch(vkw::utils::divUp(static_cast<uint32_t>(count), 256));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<float> result(count);
    if(!outputBuffer.copyToHost(result.data(), count)) { return false; }

    for(size_t i = 0; i < count; ++i)
    {
        const float expected
            = enable ? (input[i] * static_cast<float>(mult) * static_cast<float>(sign) + bias) : input[i];
        if(result[i] != expected)
        {
            vkw::utils::Log::Error(testName, "  Specialization constant mismatch at index %zu", i);
            return false;
        }
    }

    return true;
}

bool testSpecializationConstantsDefault(const vkw::Device& device)
{ return runSpecializationConstantsTest(device, 1, 0.0f, 1, true); }

bool testSpecializationConstantsCustom(const vkw::Device& device)
{ return runSpecializationConstantsTest(device, 4, 2.5f, -1, true); }

// -----------------------------------------------------------------------------------------------------------

bool testComputePipelineMultiSetLayout(const vkw::Device& device)
{
    static constexpr uint32_t w = 8;
    static constexpr uint32_t h = 8;
    static constexpr size_t count = w * h;
    static constexpr float scaleValue = 2.0f;
    static constexpr uint32_t offsetX = 2;
    static constexpr uint32_t offsetY = 1;
    static constexpr uint32_t imgW = w + offsetX;
    static constexpr uint32_t imgH = h + offsetY;

    std::vector<float> input(count);
    TestUtils::fillPattern<float>(input.data(), count, 1.0f);

    vkw::HostToDeviceBuffer<float> inputBuffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    vkw::HostStagingBuffer<float> scaleBuffer{device, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};
    if(!inputBuffer.initialized() || !scaleBuffer.initialized()) { return false; }
    if(!inputBuffer.copyFromHost(input.data(), count)) { return false; }
    scaleBuffer[0] = scaleValue;

    const VkExtent3D extent{imgW, imgH, 1};
    vkw::DeviceImage<VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT> image{
        device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, extent,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT};
    if(!image.initialized()) { return false; }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;

    vkw::ImageView imageView{device, image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R32_SFLOAT, subresourceRange};
    if(!imageView.initialized()) { return false; }

    if(!TestUtils::changeImageLayout(device, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL))
    {
        return false;
    }

    vkw::DescriptorSetLayout setLayout0{};
    if(!setLayout0.init(device)) { return false; }
    setLayout0.addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    setLayout0.addBinding<vkw::DescriptorType::UniformBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    if(!setLayout0.create()) { return false; }

    vkw::DescriptorSetLayout setLayout1{};
    if(!setLayout1.init(device)) { return false; }
    setLayout1.addBinding<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_COMPUTE_BIT, 0);
    if(!setLayout1.create()) { return false; }

    vkw::DescriptorPool descriptorPool{};
    if(!descriptorPool.init(
           device, 2,
           {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}}))
    {
        return false;
    }

    vkw::DescriptorSet descriptorSet0{};
    if(!descriptorSet0.init(device, setLayout0, descriptorPool)) { return false; }
    descriptorSet0.bindStorageBuffer(0, 0, inputBuffer);
    descriptorSet0.bindUniformBuffer(1, 0, scaleBuffer);

    vkw::DescriptorSet descriptorSet1{};
    if(!descriptorSet1.init(device, setLayout1, descriptorPool)) { return false; }
    descriptorSet1.bindStorageImage(0, 0, imageView);

    struct Params
    {
        uint32_t width;
        uint32_t offsetX;
        uint32_t offsetY;
    };

    vkw::PipelineLayout pipelineLayout{};
    if(!pipelineLayout.init(
           device, std::vector<std::reference_wrapper<vkw::DescriptorSetLayout>>{setLayout0, setLayout1}))
    {
        return false;
    }
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    if(!pipelineLayout.create()) { return false; }

    if(pipelineLayout.descriptorSetCount() != 2)
    {
        vkw::utils::Log::Error(testName, "  Unexpected descriptor set count");
        return false;
    }

    vkw::ComputePipeline pipeline{};
    if(!pipeline.init(device, reinterpret_cast<const char*>(multiSetLayoutComp), sizeof(multiSetLayoutComp)))
    {
        return false;
    }
    if(!pipeline.createPipeline(pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    Params params = {w, offsetX, offsetY};

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet0);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout, 1, descriptorSet1);
    cmdBuffer.pushConstants(pipelineLayout, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(w / 8, h / 8);
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<float> imgData(imgW * imgH, 0.0f);
    if(!TestUtils::downloadImage<float>(device, image, imgData.data(), imgW, imgH)) { return false; }

    for(uint32_t y = 0; y < h; ++y)
    {
        for(uint32_t x = 0; x < w; ++x)
        {
            const size_t srcIdx = y * w + x;
            const size_t dstIdx = (y + offsetY) * imgW + (x + offsetX);
            const float expected = input[srcIdx] * scaleValue;
            if(imgData[dstIdx] != expected)
            {
                vkw::utils::Log::Error(testName, "  Multi-set layout mismatch at (%u, %u)", x, y);
                return false;
            }
        }
    }

    return true;
}
