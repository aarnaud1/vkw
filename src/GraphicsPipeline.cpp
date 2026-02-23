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

#include "vkw/detail/GraphicsPipeline.hpp"

#include "vkw/detail/utils.hpp"

#include <stdexcept>

namespace vkw
{
GraphicsPipeline::GraphicsPipeline(const Device& device)
{
    VKW_CHECK_BOOL_FAIL(this->init(device), "Creating graphics pipeline");
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& rhs) { *this = std::move(rhs); }

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(pipeline_, rhs.pipeline_);
    std::swap(bindingDescriptions_, rhs.bindingDescriptions_);
    std::swap(attributeDescriptions_, rhs.attributeDescriptions_);

    std::swap(viewports_, rhs.viewports_);
    std::swap(scissors_, rhs.scissors_);
    std::swap(colorBlendAttachmentStates_, rhs.colorBlendAttachmentStates_);

    std::swap(vertexInputStateInfo_, rhs.vertexInputStateInfo_);
    std::swap(inputAssemblyStateInfo_, rhs.inputAssemblyStateInfo_);
    std::swap(tessellationStateInfo_, rhs.tessellationStateInfo_);
    std::swap(viewportStateInfo_, rhs.viewportStateInfo_);
    std::swap(rasterizationStateInfo_, rhs.rasterizationStateInfo_);
    std::swap(multisamplingStateInfo_, rhs.multisamplingStateInfo_);
    std::swap(depthStencilStateInfo_, rhs.depthStencilStateInfo_);
    std::swap(colorBlendStateInfo_, rhs.colorBlendStateInfo_);
    std::swap(dynamicStateInfo_, rhs.dynamicStateInfo_);

    std::swap(moduleInfo_, rhs.moduleInfo_);

    std::swap(useMeshShaders_, rhs.useMeshShaders_);
    std::swap(useTessellation_, rhs.useTessellation_);

    std::swap(initialized_, rhs.initialized_);

    return *this;
}

GraphicsPipeline::~GraphicsPipeline() { this->clear(); }

bool GraphicsPipeline::init(const Device& device)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;

    // Add one color blend attachment by default
    colorBlendAttachmentStates_.resize(1);
    colorBlendAttachmentStates_[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                                    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentStates_[0].blendEnable = VK_FALSE;
    colorBlendAttachmentStates_[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentStates_[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentStates_[0].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentStates_[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentStates_[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentStates_[0].alphaBlendOp = VK_BLEND_OP_ADD;

    viewports_.resize(1);
    scissors_.resize(1);

    // Input assembly
    inputAssemblyStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateInfo_.pNext = nullptr;
    inputAssemblyStateInfo_.flags = 0;
    inputAssemblyStateInfo_.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateInfo_.primitiveRestartEnable = false;

    // Vertex input
    vertexInputStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateInfo_.pNext = nullptr;

    // Tessellation
    tessellationStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationStateInfo_.pNext = nullptr;

    // Viewport
    viewportStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo_.pNext = nullptr;

    // Rasterization
    rasterizationStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateInfo_.depthClampEnable = VK_FALSE;
    rasterizationStateInfo_.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateInfo_.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateInfo_.lineWidth = 1.0f;
    rasterizationStateInfo_.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateInfo_.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationStateInfo_.depthBiasEnable = VK_FALSE;
    rasterizationStateInfo_.depthBiasConstantFactor = 0.0f;
    rasterizationStateInfo_.depthBiasClamp = 0.0f;
    rasterizationStateInfo_.depthBiasSlopeFactor = 0.0f;

    // MultiSample
    multisamplingStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStateInfo_.sampleShadingEnable = VK_FALSE;
    multisamplingStateInfo_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingStateInfo_.minSampleShading = 1.0f;
    multisamplingStateInfo_.pSampleMask = nullptr;
    multisamplingStateInfo_.alphaToCoverageEnable = VK_FALSE;
    multisamplingStateInfo_.alphaToOneEnable = VK_FALSE;

    // Depth stencil
    depthStencilStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateInfo_.pNext = nullptr;
    depthStencilStateInfo_.depthTestEnable = VK_TRUE;
    depthStencilStateInfo_.depthWriteEnable = VK_TRUE;
    depthStencilStateInfo_.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateInfo_.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateInfo_.minDepthBounds = 0.0f;
    depthStencilStateInfo_.maxDepthBounds = 1.0f;
    depthStencilStateInfo_.stencilTestEnable = VK_FALSE;
    depthStencilStateInfo_.front = {};
    depthStencilStateInfo_.back = {};

    // Color blend
    colorBlendStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateInfo_.logicOpEnable = VK_FALSE;
    colorBlendStateInfo_.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateInfo_.attachmentCount = 0;
    colorBlendStateInfo_.pAttachments = nullptr;
    colorBlendStateInfo_.blendConstants[0] = 0.0f;
    colorBlendStateInfo_.blendConstants[1] = 0.0f;
    colorBlendStateInfo_.blendConstants[2] = 0.0f;
    colorBlendStateInfo_.blendConstants[3] = 0.0f;

    // Dynamic state
    dynamicStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo_.dynamicStateCount = 0;
    dynamicStateInfo_.pDynamicStates = nullptr;

    initialized_ = true;

    return true;
}

void GraphicsPipeline::clear()
{
    VKW_DELETE_VK(Pipeline, pipeline_);

    bindingDescriptions_.clear();
    attributeDescriptions_.clear();

    // Destroy shader modules if not done
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
            moduleInfo_[id].shaderModule = VK_NULL_HANDLE;
        }
    }

    specInfoList_.clear();
    stageCreateInfoList_.clear();

    for(auto& info : moduleInfo_)
    {
        info = {};
    }

    useMeshShaders_ = false;
    useTessellation_ = false;

    device_ = nullptr;
    initialized_ = false;
}

GraphicsPipeline& GraphicsPipeline::addShaderStage(
    const VkShaderStageFlagBits stage, const std::string& shaderSource)
{
    VKW_ASSERT(this->initialized());

    const int id = getStageIndex(stage);
    VKW_ASSERT(id >= 0);

    auto& info = moduleInfo_[id];
    info.used = true;
    info.shaderSource = utils::readShader(shaderSource);

    if(stage == VK_SHADER_STAGE_MESH_BIT_EXT) { useMeshShaders_ = true; }

    if(stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
       || stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
    {
        useTessellation_ = true;
    }

    return *this;
}

GraphicsPipeline& GraphicsPipeline::addShaderStage(
    const VkShaderStageFlagBits stage, const char* srcData, const size_t byteCount)
{
    VKW_ASSERT(this->initialized());

    const int id = getStageIndex(stage);
    VKW_ASSERT(id >= 0);

    auto& info = moduleInfo_[id];
    info.used = true;
    info.shaderSource.resize(byteCount);
    memcpy(info.shaderSource.data(), srcData, byteCount);

    if(stage == VK_SHADER_STAGE_MESH_BIT_EXT) { useMeshShaders_ = true; }

    if(stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
       || stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
    {
        useTessellation_ = true;
    }

    return *this;
}

GraphicsPipeline& GraphicsPipeline::addVertexBinding(
    const uint32_t binding, const uint32_t stride, const VkVertexInputRate inputRate)
{
    VKW_ASSERT(this->initialized());
    VKW_ASSERT(useMeshShaders_ == false);

    bindingDescriptions_.emplace_back(VkVertexInputBindingDescription{binding, stride, inputRate});
    return *this;
}

GraphicsPipeline& GraphicsPipeline::addVertexAttribute(
    const uint32_t location, const uint32_t binding, const VkFormat format, const uint32_t offset)
{
    VKW_ASSERT(this->initialized());
    VKW_ASSERT(useMeshShaders_ == false);

    attributeDescriptions_.emplace_back(VkVertexInputAttributeDescription{location, binding, format, offset});
    return *this;
}

bool GraphicsPipeline::createPipeline(
    const RenderPass& renderPass, const PipelineLayout& pipelineLayout, const VkPipelineCreateFlags flags,
    const uint32_t subPass)
{
    VKW_ASSERT(this->initialized());

    this->finalizePipelineStages();

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    createInfo.stageCount = static_cast<uint32_t>(stageCreateInfoList_.size());
    createInfo.pStages = stageCreateInfoList_.data();
    createInfo.pVertexInputState = useMeshShaders_ ? nullptr : &vertexInputStateInfo_;
    createInfo.pInputAssemblyState = useMeshShaders_ ? nullptr : &inputAssemblyStateInfo_;
    createInfo.pTessellationState = useTessellation_ ? nullptr : &tessellationStateInfo_;
    createInfo.pViewportState = &viewportStateInfo_;
    createInfo.pRasterizationState = &rasterizationStateInfo_;
    createInfo.pMultisampleState = &multisamplingStateInfo_;
    createInfo.pDepthStencilState = &depthStencilStateInfo_;
    createInfo.pColorBlendState = &colorBlendStateInfo_;
    createInfo.pDynamicState = &dynamicStateInfo_;
    createInfo.layout = pipelineLayout.getHandle();
    createInfo.renderPass = renderPass.getHandle();
    createInfo.subpass = subPass;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;

    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateGraphicsPipelines(
        device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_));

    // Destroy shader modules
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
            moduleInfo_[id].shaderModule = VK_NULL_HANDLE;
        }
    }
    specInfoList_.clear();
    stageCreateInfoList_.clear();

    return true;
}

bool GraphicsPipeline::createPipeline(
    const PipelineLayout& pipelineLayout, const std::vector<VkFormat>& colorFormats,
    const VkFormat depthFormat, const VkFormat stencilFormat, const VkPipelineCreateFlags flags,
    const uint32_t viewMask)
{
    VKW_ASSERT(this->initialized());

    this->finalizePipelineStages();

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.pNext = nullptr;
    pipelineRenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
    pipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats.data();
    pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
    pipelineRenderingCreateInfo.stencilAttachmentFormat = stencilFormat;
    pipelineRenderingCreateInfo.viewMask = viewMask;

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.flags = flags;
    createInfo.stageCount = static_cast<uint32_t>(stageCreateInfoList_.size());
    createInfo.pStages = stageCreateInfoList_.data();
    createInfo.pVertexInputState = useMeshShaders_ ? nullptr : &vertexInputStateInfo_;
    createInfo.pInputAssemblyState = useMeshShaders_ ? nullptr : &inputAssemblyStateInfo_;
    createInfo.pTessellationState = useTessellation_ ? &tessellationStateInfo_ : nullptr;
    createInfo.pViewportState = &viewportStateInfo_;
    createInfo.pRasterizationState = &rasterizationStateInfo_;
    createInfo.pMultisampleState = &multisamplingStateInfo_;
    createInfo.pDepthStencilState = &depthStencilStateInfo_;
    createInfo.pColorBlendState = &colorBlendStateInfo_;
    createInfo.pDynamicState = &dynamicStateInfo_;
    createInfo.layout = pipelineLayout.getHandle();
    createInfo.renderPass = VK_NULL_HANDLE;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;
    createInfo.pNext = &pipelineRenderingCreateInfo;

    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateGraphicsPipelines(
        device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_));

    // Destroy shader modules
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
            moduleInfo_[id].shaderModule = VK_NULL_HANDLE;
        }
    }
    specInfoList_.clear();
    stageCreateInfoList_.clear();

    return true;
}

void GraphicsPipeline::finalizePipelineStages()
{
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        auto& info = moduleInfo_[id];
        if(info.used)
        {
            info.shaderModule
                = utils::createShaderModule(device_->vk(), device_->getHandle(), info.shaderSource);
        }
    }

    for(size_t id = 0; id < maxStageCount; ++id)
    {
        size_t offset = 0;
        auto& specMap = specMaps_[id];
        for(size_t i = 0; i < moduleInfo_[id].specSizes.size(); i++)
        {
            VkSpecializationMapEntry mapEntry
                = {static_cast<uint32_t>(i), static_cast<uint32_t>(offset), moduleInfo_[id].specSizes[i]};
            specMap.push_back(mapEntry);
            offset += moduleInfo_[id].specSizes[i];
        }
    }

    /// @note: Pre-allocate data to avoid reallocation
    specInfoList_.resize(maxStageCount);
    stageCreateInfoList_.reserve(maxStageCount);

    size_t index = 0;
    uint32_t stageCount = 0;
    auto addShaderSpecInfo = [&](const auto stage) {
        const int id = getStageIndex(stage);
        if(moduleInfo_[id].shaderModule == VK_NULL_HANDLE) { return; }
        auto& specMap = specMaps_[id];
        auto& specSizes = moduleInfo_[id].specSizes;
        auto& specData = moduleInfo_[id].specData;

        if(specSizes.size() > 0)
        {
            specInfoList_[index].mapEntryCount = static_cast<uint32_t>(specMap.size());
            specInfoList_[index].pMapEntries = specMap.data();
            specInfoList_[index].dataSize = static_cast<uint32_t>(specData.size());
            specInfoList_[index].pData = specData.data();
        }

        VkPipelineShaderStageCreateInfo stageCreateInfo{};
        stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCreateInfo.pNext = nullptr;
        stageCreateInfo.flags = 0;
        stageCreateInfo.stage = stage;
        stageCreateInfo.module = moduleInfo_[id].shaderModule;
        stageCreateInfo.pName = "main";
        stageCreateInfo.pSpecializationInfo = specSizes.size() > 0 ? &specInfoList_[index] : nullptr;

        stageCreateInfoList_.emplace_back(stageCreateInfo);

        if(specSizes.size() > 0) { index++; }
        stageCount++;
    };
    addShaderSpecInfo(VK_SHADER_STAGE_VERTEX_BIT);
    addShaderSpecInfo(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    addShaderSpecInfo(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
    addShaderSpecInfo(VK_SHADER_STAGE_GEOMETRY_BIT);
    addShaderSpecInfo(VK_SHADER_STAGE_FRAGMENT_BIT);
    addShaderSpecInfo(VK_SHADER_STAGE_TASK_BIT_EXT);
    addShaderSpecInfo(VK_SHADER_STAGE_MESH_BIT_EXT);

    // Viewport
    viewportStateInfo_.viewportCount = static_cast<uint32_t>(viewports_.size());
    viewportStateInfo_.pViewports = viewports_.data();
    viewportStateInfo_.scissorCount = static_cast<uint32_t>(scissors_.size());
    viewportStateInfo_.pScissors = scissors_.data();

    // Vertex input
    if(!useMeshShaders_)
    {
        vertexInputStateInfo_.flags = 0;
        vertexInputStateInfo_.vertexBindingDescriptionCount
            = static_cast<uint32_t>(bindingDescriptions_.size());
        vertexInputStateInfo_.pVertexBindingDescriptions = bindingDescriptions_.data();
        vertexInputStateInfo_.vertexAttributeDescriptionCount
            = static_cast<uint32_t>(attributeDescriptions_.size());
        vertexInputStateInfo_.pVertexAttributeDescriptions = attributeDescriptions_.data();
    }

    colorBlendStateInfo_.attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates_.size());
    colorBlendStateInfo_.pAttachments = colorBlendAttachmentStates_.data();

    // Dynamic states
    dynamicStateInfo_.dynamicStateCount = static_cast<uint32_t>(dynamicStates_.size());
    dynamicStateInfo_.pDynamicStates = dynamicStates_.data();
}
} // namespace vkw
