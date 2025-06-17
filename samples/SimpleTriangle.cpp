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

#include "SimpleTriangle.hpp"

#include "Common.hpp"

SimpleTriangle::SimpleTriangle(
    const uint32_t frameWidth,
    const uint32_t frameHeight,
    const std::vector<const char*>& instanceExtensions)
    : IGraphicsSample(frameWidth, frameHeight, instanceExtensions)
    , fboWidth_{frameWidth}
    , fboHeight_{frameHeight}
{
    // Dynamic rendering
    dynamicRenderingFeatures_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures_.pNext = nullptr;
    dynamicRenderingFeatures_.dynamicRendering = VK_TRUE;

    deviceFeatures_.pNext = &dynamicRenderingFeatures_;

    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkPhysicalDevice SimpleTriangle::findSupportedDevice() const
{
    return findCompatibleDevice(instance_, deviceExtensions_, dynamicRenderingFeatures_);
}

bool SimpleTriangle::init()
{
    static constexpr uint32_t vetexCount = 3;
    static const glm::vec3 positions[vetexCount]
        = {{0.0f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}};
    static const glm::vec3 colors[vetexCount]
        = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

    // Initialize resources
    positions_.init(
        device_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vetexCount);
    colors_.init(
        device_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, vetexCount);

    // Init graphics pipeline
    pipelineLayout_.init(device_);
    pipelineLayout_.create();

    graphicsPipeline_.init(device_);
    graphicsPipeline_.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "build/spv/triangle.vert.spv");
    graphicsPipeline_.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "build/spv/triangle.frag.spv");
    graphicsPipeline_.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    graphicsPipeline_.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
    graphicsPipeline_.addVertexBinding(0, sizeof(glm::vec3))
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);
    graphicsPipeline_.addVertexBinding(1, sizeof(glm::vec3))
        .addVertexAttribute(1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0);
    graphicsPipeline_.multisamplingStateInfo().rasterizationSamples = sampleCount;
    graphicsPipeline_.createPipeline(pipelineLayout_, {colorFormat});

    // Stream vertices
    uploadData(device_, positions, positions_);
    uploadData(device_, colors, colors_);

    for(uint32_t i = 0; i < IGraphicsSample::framesInFlight; ++i)
    {
        vkw::DeviceImage<> image(
            device_,
            VK_IMAGE_TYPE_2D,
            colorFormat,
            {fboWidth_, fboHeight_, 1},
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            sampleCount);
        vkw::ImageView imageView(
            device_,
            image,
            VK_IMAGE_VIEW_TYPE_2D,
            colorFormat,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        fboColorImages_.emplace_back(std::move(image));
        fboColorImageViews_.emplace_back(std::move(imageView));

        vkw::DeviceImage<> resolveImage(
            device_,
            VK_IMAGE_TYPE_2D,
            colorFormat,
            {fboWidth_, fboHeight_, 1},
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        vkw::ImageView resolveImageView(
            device_,
            resolveImage,
            VK_IMAGE_VIEW_TYPE_2D,
            colorFormat,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
        fboResolveImages_.emplace_back(std::move(resolveImage));
        fboResolveImageViews_.emplace_back(std::move(resolveImageView));
    }
    initFboImageLayouts();

    return true;
}

bool SimpleTriangle::recordInitCommands(
    vkw::CommandBuffer& /*cmdBuffer*/, const uint32_t /*frameId*/)
{
    return false;
}

void SimpleTriangle::recordDrawCommands(
    vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId)
{
    VkClearValue clearColor = {};
    clearColor.color = {0.1f, 0.1f, 0.1f, 1.0f};

    vkw::RenderingAttachment colorAttachment{
        fboColorImageViews_[frameId],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        fboResolveImageViews_[frameId],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_RESOLVE_MODE_AVERAGE_BIT,
        clearColor,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE};

    cmdBuffer.reset();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    {
        cmdBuffer.beginRendering(colorAttachment, {0, 0, fboWidth_, fboHeight_});
        cmdBuffer.bindGraphicsPipeline(graphicsPipeline_);
        cmdBuffer.setViewport(0.0f, 0.0f, float(fboWidth_), float(fboHeight_));
        cmdBuffer.setScissor({0, 0}, {fboWidth_, fboHeight_});
        cmdBuffer.bindVertexBuffer(0, positions_, 0);
        cmdBuffer.bindVertexBuffer(1, colors_, 0);
        cmdBuffer.draw(3, 1, 0, 0);
        cmdBuffer.endRendering();

        cmdBuffer.imageMemoryBarrier(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createImageMemoryBarrier(
                fboResolveImages_[frameId],
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL));
        cmdBuffer.imageMemoryBarrier(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createImageMemoryBarrier(
                swapchain_.images()[imageId],
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

        VkImageBlit region = {};
        region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.srcOffsets[0] = {0, 0, 0};
        region.srcOffsets[1] = {int(fboWidth_), int(fboHeight_), 1};
        region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        region.dstOffsets[0] = {0, 0, 0};
        region.dstOffsets[1] = {int(frameWidth_), int(frameHeight_), 1};
        cmdBuffer.blitImage(
            fboResolveImages_[frameId].getHandle(),
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchain_.images()[imageId],
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            region);
        cmdBuffer.imageMemoryBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            vkw::createImageMemoryBarrier(
                swapchain_.images()[imageId],
                VK_ACCESS_TRANSFER_WRITE_BIT,
                0,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
        cmdBuffer.imageMemoryBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            vkw::createImageMemoryBarrier(
                fboResolveImages_[frameId],
                VK_ACCESS_TRANSFER_READ_BIT,
                0,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
    }
    cmdBuffer.end();
}

bool SimpleTriangle::recordPostDrawCommands(
    vkw::CommandBuffer& /*cmdBuffer*/, const uint32_t /*frameId*/, const uint32_t /*imageId*/)
{
    return false;
}

bool SimpleTriangle::postDraw() { return true; }

void SimpleTriangle::initFboImageLayouts()
{
    auto cmdBuffer = cmdPool_.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // Initialize swapchain images
    std::vector<VkImageMemoryBarrier> memoryBarriers{};

    for(uint32_t i = 0; i < IGraphicsSample::framesInFlight; ++i)
    {
        VkImageMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = 0;
        memoryBarrier.image = fboColorImages_[i].getHandle();
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        memoryBarriers.push_back(memoryBarrier);
    }

    for(uint32_t i = 0; i < IGraphicsSample::framesInFlight; ++i)
    {
        VkImageMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = 0;
        memoryBarrier.image = fboResolveImages_[i].getHandle();
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        memoryBarriers.push_back(memoryBarrier);
    }

    cmdBuffer.imageMemoryBarriers(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, memoryBarriers);
    cmdBuffer.end();

    auto initFence = vkw::Fence(device_, false);
    graphicsQueue_.submit(cmdBuffer, initFence);
    initFence.wait();
}