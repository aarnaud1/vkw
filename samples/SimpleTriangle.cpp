/*
 * Copyright (C) 2025 Adrien ARNAUD
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
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