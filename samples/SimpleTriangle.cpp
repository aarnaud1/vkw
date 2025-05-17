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

SimpleTriangle::SimpleTriangle()
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
    graphicsPipeline_.createPipeline(pipelineLayout_, {colorFormat});

    // Stream vertices
    uploadData(device_, positions, positions_);
    uploadData(device_, colors, colors_);

    return true;
}

bool SimpleTriangle::recordInitCommands(
    vkw::CommandBuffer& /*cmdBuffer*/, const uint32_t /*frameId*/)
{
    return false;
}

void SimpleTriangle::recordDrawCommands(
    vkw::CommandBuffer& cmdBuffer, const uint32_t /*frameId*/, const uint32_t imageId)
{
    VkClearValue clearColor = {};
    clearColor.color = {0.1f, 0.1f, 0.1f, 1.0f};

    vkw::RenderingAttachment colorAttachment{
        swapchain_.imageView(imageId),
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        clearColor,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE};

    cmdBuffer.reset();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    {
        cmdBuffer.beginRendering(colorAttachment, {0, 0, frameWidth_, frameHeight_});
        cmdBuffer.bindGraphicsPipeline(graphicsPipeline_);
        cmdBuffer.setViewport(0.0f, 0.0f, float(frameWidth_), float(frameHeight_));
        cmdBuffer.setScissor({0, 0}, {frameWidth_, frameHeight_});
        cmdBuffer.bindVertexBuffer(0, positions_, 0);
        cmdBuffer.bindVertexBuffer(1, colors_, 0);
        cmdBuffer.draw(3, 1, 0, 0);
        cmdBuffer.endRendering();
    }
    cmdBuffer.end();
}

bool SimpleTriangle::recordPostDrawCommands(
    vkw::CommandBuffer& /*cmdBuffer*/, const uint32_t /*frameId*/, const uint32_t /*imageId*/)
{
    return false;
}

bool SimpleTriangle::postDraw() { return true; }