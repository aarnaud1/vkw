/*
 * Copyright (C) 2024 Adrien ARNAUD
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

#include "vkWrappers/wrappers/GraphicsPipeline.hpp"

#include <stdexcept>

namespace vk
{
GraphicsPipeline::GraphicsPipeline(Device& device) : device_{&device} {}

GraphicsPipeline& GraphicsPipeline::addShaderStage(
    const VkShaderStageFlagBits stage, const std::string& shaderSource)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding shaders to a created pipeline");
    }

    const int id = getStageIndex(stage);
    if(id < 0)
    {
        throw std::runtime_error("Unsupported shader stage for graphics pipeline");
    }

    auto& info = moduleInfo_[id];
    if(info.shaderModule != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Shader stage already created for this pipeline");
    }
    info.shaderModule
        = utils::createShaderModule(device_->getHandle(), utils::readShader(shaderSource));

    return *this;
}

GraphicsPipeline& GraphicsPipeline::addVertexBinding(
    const uint32_t binding, const uint32_t stride, const VkVertexInputRate inputRate)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding vertex binding to a created pipeline");
    }
    bindingDescriptions_.emplace_back(VkVertexInputBindingDescription{binding, stride, inputRate});
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addVertexAttribute(
    const uint32_t location, const uint32_t binding, const VkFormat format, const uint32_t offset)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding vertex attribute to a created pipeline");
    }
    attributeDescriptions_.emplace_back(
        VkVertexInputAttributeDescription{location, binding, format, offset});
    return *this;
}

GraphicsPipeline& GraphicsPipeline::setPrimitiveType(
    const VkPrimitiveTopology primitive, const VkBool32 primitiveEnableRestart)
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding topology to a created pipeline");
    }
    topology_ = primitive;
    primitiveEnableRestart_ = primitiveEnableRestart;
    return *this;
}

void GraphicsPipeline::createPipeline(
    RenderPass& renderPass, PipelineLayout& pipelineLayout, const uint32_t subPass)
{
    std::array<std::vector<VkSpecializationMapEntry>, maxStageCount> specMaps;
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        size_t offset = 0;
        auto& specMap = specMaps[id];
        for(size_t i = 0; i < moduleInfo_[id].specSizes.size(); i++)
        {
            VkSpecializationMapEntry mapEntry
                = {static_cast<uint32_t>(i),
                   static_cast<uint32_t>(offset),
                   moduleInfo_[id].specSizes[i]};
            specMap.push_back(mapEntry);
            offset += moduleInfo_[id].specSizes[i];
        }
    }

    std::vector<VkSpecializationInfo> specInfoList;
    std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfoList;

    // Important : pre allocate data to avoid reallocation
    specInfoList.reserve(maxStageCount);
    stageCreateInfoList.reserve(maxStageCount);

    auto addShaderSpecInfo = [&](const int id, const auto stage) {
        if(moduleInfo_[id].shaderModule == VK_NULL_HANDLE)
        {
            return;
        }
        auto& specMap = specMaps[id];
        auto& specData = moduleInfo_[id].specData;

        VkSpecializationInfo specInfo{};
        specInfo.mapEntryCount = specMap.size();
        specInfo.pMapEntries = specMap.data();
        specInfo.pData = specData.data();
        specInfo.dataSize = specData.size();

        specInfoList.emplace_back(specInfo);

        auto& specSizes = moduleInfo_[id].specSizes;

        VkPipelineShaderStageCreateInfo stageCreateInfo{};
        stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCreateInfo.pNext = nullptr;
        stageCreateInfo.flags = 0;
        stageCreateInfo.stage = stage;
        stageCreateInfo.module = moduleInfo_[id].shaderModule;
        stageCreateInfo.pName = "main";
        stageCreateInfo.pSpecializationInfo = specSizes.size() > 0 ? &specInfo : nullptr;

        stageCreateInfoList.emplace_back(stageCreateInfo);
    };
    addShaderSpecInfo(0, VK_SHADER_STAGE_VERTEX_BIT);
    addShaderSpecInfo(1, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    addShaderSpecInfo(2, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    addShaderSpecInfo(3, VK_SHADER_STAGE_GEOMETRY_BIT);
    addShaderSpecInfo(4, VK_SHADER_STAGE_FRAGMENT_BIT);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
    vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateInfo.pNext = nullptr;
    vertexInputStateInfo.flags = 0;
    vertexInputStateInfo.vertexBindingDescriptionCount
        = static_cast<uint32_t>(bindingDescriptions_.size());
    vertexInputStateInfo.pVertexBindingDescriptions = bindingDescriptions_.data();
    vertexInputStateInfo.vertexAttributeDescriptionCount
        = static_cast<uint32_t>(attributeDescriptions_.size());
    vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions_.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
    inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateInfo.pNext = nullptr;
    inputAssemblyStateInfo.flags = 0;
    inputAssemblyStateInfo.topology = topology_;
    inputAssemblyStateInfo.primitiveRestartEnable = primitiveEnableRestart_;

    // Tesselation
    // TODO : add this

    // Viewport
    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport_;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor_;

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationStateInfo{};
    rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateInfo.lineWidth = 1.0f;
    rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationStateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationStateInfo.depthBiasClamp = 0.0f;
    rasterizationStateInfo.depthBiasSlopeFactor = 0.0f;

    // MultiSample
    VkPipelineMultisampleStateCreateInfo multisamplingStateInfo{};
    multisamplingStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingStateInfo.minSampleShading = 1.0f;
    multisamplingStateInfo.pSampleMask = nullptr;
    multisamplingStateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingStateInfo.alphaToOneEnable = VK_FALSE;

    // Depth stencil
    // TODO : add this

    // Color blend
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                               | VK_COLOR_COMPONENT_B_BIT
                                               | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachmentState;
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    // Dynamic state
    std::vector<VkDynamicState> dynamicStates
        = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    // createInfo.flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT;
    createInfo.stageCount = static_cast<uint32_t>(stageCreateInfoList.size());
    createInfo.pStages = stageCreateInfoList.data();
    createInfo.pVertexInputState = &vertexInputStateInfo;
    createInfo.pInputAssemblyState = &inputAssemblyStateInfo;
    createInfo.pTessellationState = nullptr;
    createInfo.pViewportState = &viewportStateInfo;
    createInfo.pRasterizationState = &rasterizationStateInfo;
    createInfo.pMultisampleState = &multisamplingStateInfo;
    createInfo.pDepthStencilState = nullptr;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDynamicState = &dynamicState;
    createInfo.layout = pipelineLayout.getHandle();
    createInfo.renderPass = renderPass.getHandle();
    createInfo.subpass = subPass;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;

    CHECK_VK(
        vkCreateGraphicsPipelines(
            device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_),
        "Creating graphics pipeline");
}

GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(device_->getHandle(), pipeline_, nullptr);
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
        }
    }
}
} // namespace vk