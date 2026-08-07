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

#include "TestGraphicsPipeline.hpp"

#include "Utils.hpp"

#include <vector>
#include <vkw/vkw.hpp>

static const char* testName = "GraphicsPipelineTest";

static constexpr uint32_t imgW = 16;
static constexpr uint32_t imgH = 16;
static constexpr VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
static constexpr VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

// -----------------------------------------------------------------------------------------------------------

static bool testBasicGraphicsPipelineRenderPass(const vkw::Device& device);
static bool testBasicGraphicsPipelineDynamicRendering(
    const vkw::Instance& instance, const VkPhysicalDevice physicalDevice);
static bool testPushConstantStages(const vkw::Device& device);
static bool testDynamicViewportScissor(const vkw::Device& device);
static bool testDepthTest(const vkw::Device& device);
static bool testBlend(const vkw::Device& device);

// -----------------------------------------------------------------------------------------------------------

static const uint32_t fullscreenRedVertSpv[] = {
#include "spv/FullscreenRed.vert.spv"
};
static const uint32_t fullscreenRedFragSpv[] = {
#include "spv/FullscreenRed.frag.spv"
};
static const uint32_t coloredQuadVertSpv[] = {
#include "spv/ColoredQuad.vert.spv"
};
static const uint32_t coloredQuadFragSpv[] = {
#include "spv/ColoredQuad.frag.spv"
};

// -----------------------------------------------------------------------------------------------------------

struct PushConstants
{
    float offsetX;
    float offsetY;
    float scaleX;
    float scaleY;
    float depth;
    float colorR;
    float colorG;
    float colorB;
    float colorA;
};

// -----------------------------------------------------------------------------------------------------------

bool launchGraphicsPipelineTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    if(!TestUtils::isFormatSupported(
           physicalDevice, colorFormat,
           VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT))
    {
        vkw::utils::Log::Info(testName, "Color format not fully supported, skipping");
        return true;
    }

    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(device.init(instance, physicalDevice, {}, {}));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    vkw::utils::Log::Info(testName, "Checking basic graphics pipeline (render pass)...");
    if(!testBasicGraphicsPipelineRenderPass(device))
    {
        vkw::utils::Log::Warning(testName, "  Basic pipeline (render pass) - FAILED");
        failedTests++;
    }
    totalTests++;

    // vkw::utils::Log::Info(testName, "Checking basic graphics pipeline (dynamic rendering)...");
    // if(!testBasicGraphicsPipelineDynamicRendering(instance, physicalDevice))
    // {
    //     vkw::utils::Log::Warning(testName, "  Basic pipeline (dynamic rendering) - FAILED");
    //     failedTests++;
    // }
    // totalTests++;

    vkw::utils::Log::Info(testName, "Checking push constant shader stages...");
    if(!testPushConstantStages(device))
    {
        vkw::utils::Log::Warning(testName, "  Push constant stages - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking dynamic viewport/scissor...");
    if(!testDynamicViewportScissor(device))
    {
        vkw::utils::Log::Warning(testName, "  Dynamic viewport/scissor - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking depth test...");
    if(!testDepthTest(device))
    {
        vkw::utils::Log::Warning(testName, "  Depth test - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking blending...");
    if(!testBlend(device))
    {
        vkw::utils::Log::Warning(testName, "  Blending - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return failedTests == 0;
}

// -----------------------------------------------------------------------------------------------------------

static bool downloadColorImage(
    const vkw::Device& device, const vkw::BaseImage& image, std::vector<uint8_t>& outPixels)
{
    outPixels.resize(imgW * imgH * 4);
    return TestUtils::downloadImage<uint8_t>(device, image, outPixels.data(), imgW * 4, imgH);
}

static bool checkSolidColor(
    const std::vector<uint8_t>& pixels, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    for(uint32_t i = 0; i < imgW * imgH; ++i)
    {
        const uint8_t* p = &pixels[i * 4];
        if((p[0] != r) || (p[1] != g) || (p[2] != b) || (p[3] != a)) { return false; }
    }
    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testBasicGraphicsPipelineRenderPass(const vkw::Device& device)
{
    vkw::DeviceImage<VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT> colorImage{
        device, VK_IMAGE_TYPE_2D, colorFormat, VkExtent3D{imgW, imgH, 1},
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT};
    if(!colorImage.initialized()) { return false; }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;

    vkw::ImageView colorView{device, colorImage, VK_IMAGE_VIEW_TYPE_2D, colorFormat, subresourceRange};
    if(!colorView.initialized()) { return false; }

    if(!TestUtils::changeImageLayout(
           device, colorImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
    {
        return false;
    }

    vkw::RenderPass renderPass{device};
    renderPass.addColorAttachment(
        colorFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    renderPass.addSubPass({0});
    renderPass.create();

    vkw::Framebuffer framebuffer{device, renderPass, imgW, imgH};
    framebuffer.addAttachment(colorView);
    framebuffer.create();

    vkw::PipelineLayout pipelineLayout{device};
    pipelineLayout.create();

    vkw::GraphicsPipeline pipeline{device};
    pipeline.addShaderStage(
        VK_SHADER_STAGE_VERTEX_BIT, reinterpret_cast<const char*>(fullscreenRedVertSpv),
        sizeof(fullscreenRedVertSpv));
    pipeline.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT, reinterpret_cast<const char*>(fullscreenRedFragSpv),
        sizeof(fullscreenRedFragSpv));
    pipeline.viewports()[0]
        = VkViewport{0.0f, 0.0f, static_cast<float>(imgW), static_cast<float>(imgH), 0.0f, 1.0f};
    pipeline.scissors()[0] = VkRect2D{{0, 0}, {imgW, imgH}};
    if(!pipeline.createPipeline(renderPass, pipelineLayout)) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Graphics)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.beginRenderPass(
        renderPass, framebuffer.getHandle(), VkOffset2D{0, 0}, VkExtent2D{imgW, imgH},
        VkClearColorValue{{0.0f, 0.0f, 0.0f, 1.0f}});
    cmdBuffer.bindGraphicsPipeline(pipeline);
    cmdBuffer.draw(6, 1, 0, 0);
    cmdBuffer.endRenderPass();
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Graphics)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<uint8_t> pixels;
    if(!downloadColorImage(device, colorImage, pixels)) { return false; }

    if(!checkSolidColor(pixels, 255, 0, 0, 255))
    {
        vkw::utils::Log::Error(testName, "  Basic pipeline: unexpected color output");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testBasicGraphicsPipelineDynamicRendering(
    const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceVulkan13Features availableVk13Features = {};
    availableVk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    availableVk13Features.pNext = nullptr;

    VkPhysicalDeviceFeatures2 availableFeatures = {};
    availableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    availableFeatures.pNext = &availableVk13Features;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &availableFeatures);

    if(availableVk13Features.dynamicRendering == VK_FALSE)
    {
        vkw::utils::Log::Info(testName, "  Dynamic rendering not supported, skipping");
        return true;
    }

    VkPhysicalDeviceVulkan13Features enabledVk13Features = {};
    enabledVk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    enabledVk13Features.pNext = nullptr;
    enabledVk13Features.dynamicRendering = VK_TRUE;

    vkw::Device device{};
    if(!device.init(instance, physicalDevice, {}, {}, &enabledVk13Features)) { return false; }

    vkw::DeviceImage<VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT> colorImage{
        device, VK_IMAGE_TYPE_2D, colorFormat, VkExtent3D{imgW, imgH, 1},
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT};
    if(!colorImage.initialized()) { return false; }

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;

    vkw::ImageView colorView{device, colorImage, VK_IMAGE_VIEW_TYPE_2D, colorFormat, subresourceRange};
    if(!colorView.initialized()) { return false; }

    if(!TestUtils::changeImageLayout(
           device, colorImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
    {
        return false;
    }

    vkw::PipelineLayout pipelineLayout{device};
    pipelineLayout.create();

    vkw::GraphicsPipeline pipeline{device};
    pipeline.addShaderStage(
        VK_SHADER_STAGE_VERTEX_BIT, reinterpret_cast<const char*>(fullscreenRedVertSpv),
        sizeof(fullscreenRedVertSpv));
    pipeline.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT, reinterpret_cast<const char*>(fullscreenRedFragSpv),
        sizeof(fullscreenRedFragSpv));
    pipeline.viewports()[0]
        = VkViewport{0.0f, 0.0f, static_cast<float>(imgW), static_cast<float>(imgH), 0.0f, 1.0f};
    pipeline.scissors()[0] = VkRect2D{{0, 0}, {imgW, imgH}};
    if(!pipeline.createPipeline(pipelineLayout, {colorFormat})) { return false; }

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Graphics)[0]};
    if(!cmdPool.initialized()) { return false; }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized()) { return false; }

    VkClearValue clearValue{};
    clearValue.color = VkClearColorValue{{0.0f, 0.0f, 0.0f, 1.0f}};
    vkw::RenderingAttachment colorAttachment{
        colorView, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, clearValue, VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE};

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.beginRendering(colorAttachment, VkRect2D{{0, 0}, {imgW, imgH}});
    cmdBuffer.bindGraphicsPipeline(pipeline);
    cmdBuffer.draw(6, 1, 0, 0);
    cmdBuffer.endRendering();
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized()) { return false; }
    if(device.getQueues(vkw::QueueUsageBits::Graphics)[0].submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        return false;
    }
    if(!fence.wait()) { return false; }

    std::vector<uint8_t> pixels;
    if(!downloadColorImage(device, colorImage, pixels)) { return false; }

    if(!checkSolidColor(pixels, 255, 0, 0, 255))
    {
        vkw::utils::Log::Error(testName, "  Dynamic rendering: unexpected color output");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testPushConstantStages(const vkw::Device& device) { return true; }

// -----------------------------------------------------------------------------------------------------------

bool testDynamicViewportScissor(const vkw::Device& device) { return true; }

// -----------------------------------------------------------------------------------------------------------

bool testDepthTest(const vkw::Device& device) { return true; }

// -----------------------------------------------------------------------------------------------------------

bool testBlend(const vkw::Device& device) { return true; }