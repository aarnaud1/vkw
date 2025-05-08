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

#include "vkw/wrappers/GraphicsPipeline.hpp"

#include "vkw/wrappers/utils.hpp"

#include <stdexcept>

namespace vkw
{
GraphicsPipeline::GraphicsPipeline(Device& device)
{
    VKW_CHECK_BOOL_THROW(this->init(device), "Creating graphics pipeline");
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& cp) { *this = std::move(cp); }

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(pipeline_, cp.pipeline_);
    std::swap(bindingDescriptions_, cp.bindingDescriptions_);
    std::swap(attributeDescriptions_, cp.attributeDescriptions_);

    std::swap(viewports_, cp.viewports_);
    std::swap(scissors_, cp.scissors_);
    std::swap(colorBlendAttachmentStates_, cp.colorBlendAttachmentStates_);

    std::swap(vertexInputStateInfo_, cp.vertexInputStateInfo_);
    std::swap(inputAssemblyStateInfo_, cp.inputAssemblyStateInfo_);
    std::swap(tessellationStateInfo_, cp.tessellationStateInfo_);
    std::swap(viewportStateInfo_, cp.viewportStateInfo_);
    std::swap(rasterizationStateInfo_, cp.rasterizationStateInfo_);
    std::swap(multisamplingStateInfo_, cp.multisamplingStateInfo_);
    std::swap(depthStencilStateInfo_, cp.depthStencilStateInfo_);
    std::swap(colorBlendStateInfo_, cp.colorBlendStateInfo_);
    std::swap(dynamicStateInfo_, cp.dynamicStateInfo_);

    std::swap(moduleInfo_, cp.moduleInfo_);

    std::swap(useMeshShaders_, cp.useMeshShaders_);
    std::swap(useTessellation_, cp.useTessellation_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

GraphicsPipeline::~GraphicsPipeline() { this->clear(); }

bool GraphicsPipeline::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;

        // Add one color blend attachment by default
        colorBlendAttachmentStates_.resize(1);
        colorBlendAttachmentStates_[0].colorWriteMask
            = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
              | VK_COLOR_COMPONENT_A_BIT;
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
    }

    return true;
}

void GraphicsPipeline::clear()
{
    VKW_DELETE_VK(Pipeline, pipeline_);

    device_ = nullptr;

    bindingDescriptions_.clear();
    attributeDescriptions_.clear();

    // Destroy shader modules if not done
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(
                device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
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

    initialized_ = false;
}

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
    info.used = true;
    info.shaderSource = utils::readShader(shaderSource);

    if(stage == VK_SHADER_STAGE_MESH_BIT_EXT)
    {
        useMeshShaders_ = true;
    }

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
    info.used = true;
    info.shaderSource.resize(byteCount);
    memcpy(info.shaderSource.data(), srcData, byteCount);

    if(stage == VK_SHADER_STAGE_MESH_BIT_EXT)
    {
        useMeshShaders_ = true;
    }

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
    if(pipeline_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Adding vertex binding to a created pipeline");
    }
    if(useMeshShaders_)
    {
        throw std::runtime_error("Input assembly disabled with mesh shaders");
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
    if(useMeshShaders_)
    {
        throw std::runtime_error("Input assembly disabled with mesh shaders");
    }
    attributeDescriptions_.emplace_back(
        VkVertexInputAttributeDescription{location, binding, format, offset});
    return *this;
}

void GraphicsPipeline::createPipeline(
    RenderPass& renderPass, PipelineLayout& pipelineLayout, const uint32_t subPass)
{
    this->finalizePipelineStages();

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
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

    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateGraphicsPipelines(
            device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_),
        "Creating graphics pipeline");

    // Destroy shader modules
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(
                device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
            moduleInfo_[id].shaderModule = VK_NULL_HANDLE;
        }
    }
    specInfoList_.clear();
    stageCreateInfoList_.clear();
}

void GraphicsPipeline::createPipeline(
    PipelineLayout& pipelineLayout,
    const std::vector<VkFormat>& colorFormats,
    const VkFormat depthFormat,
    const VkFormat stencilFormat,
    const uint32_t viewMask)
{
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
    createInfo.renderPass = VK_NULL_HANDLE;
    createInfo.subpass = 0;
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;
    createInfo.pNext = &pipelineRenderingCreateInfo;

    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateGraphicsPipelines(
            device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_),
        "Creating graphics pipeline");

    // Destroy shader modules
    for(size_t id = 0; id < maxStageCount; ++id)
    {
        if(moduleInfo_[id].shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(
                device_->getHandle(), moduleInfo_[id].shaderModule, nullptr);
            moduleInfo_[id].shaderModule = VK_NULL_HANDLE;
        }
    }
    specInfoList_.clear();
    stageCreateInfoList_.clear();
}

bool GraphicsPipeline::validatePipeline()
{
    const bool hasVertexShader = moduleInfo_[0].used;
    const bool hasTessellationControlShader = moduleInfo_[1].used;
    const bool hasTessellationEvaluationShader = moduleInfo_[2].used;
    const bool hasGeometryShader = moduleInfo_[3].used;
    const bool hasFragmentShader = moduleInfo_[4].used;
    const bool hasTaskShader = moduleInfo_[5].used;
    const bool hasMeshShader = moduleInfo_[6].used;

    if(useMeshShaders_ && useTessellation_)
    {
        utils::Log::Error("vkw", "Tessellation set with mesh shaders");
        return false;
    }

    if(useMeshShaders_)
    {
        if(hasVertexShader || hasTessellationControlShader || hasTessellationEvaluationShader
           || hasGeometryShader)
        {
            utils::Log::Error(
                "vkw",
                "With mesh shaders, vertex, tessellation and geometry bshaders cannopt be used");
            return false;
        }

        if(!hasMeshShader)
        {
            utils::Log::Error("vkw", "Mesh shader pipeline must define a mesh shader");
            return false;
        }
    }
    else
    {
        if(!hasVertexShader)
        {
            utils::Log::Error("vkw", "Graphics pipeline must define a vertex shader");
            return false;
        }

        if(hasMeshShader || hasTaskShader)
        {
            utils::Log::Error(
                "vkw", "Traditional graphics pipeline must not define task or mesh shaders");
            return false;
        }

        if(useTessellation_)
        {
            if(!hasTessellationEvaluationShader)
            {
                utils::Log::Error("vkw", "Tessellation enabled but no tessellation shader");
                return false;
            }
        }
    }

    if(!hasFragmentShader)
    {
        utils::Log::Error("vkw", "Graphics pipeline must have a fragment shader");
        return false;
    }

    return true;
}

void GraphicsPipeline::finalizePipelineStages()
{
    // Make some pre checks to avoid mixing traditional pipeline and mesh pipeline
    VKW_CHECK_BOOL_THROW(validatePipeline(), "Graphics pipeline built with incompatible settings");

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
                = {static_cast<uint32_t>(i),
                   static_cast<uint32_t>(offset),
                   moduleInfo_[id].specSizes[i]};
            specMap.push_back(mapEntry);
            offset += moduleInfo_[id].specSizes[i];
        }
    }

    // Important : pre allocate data to avoid reallocation
    specInfoList_.resize(maxStageCount);
    stageCreateInfoList_.reserve(maxStageCount);

    size_t index = 0;
    uint32_t stageCount = 0;
    auto addShaderSpecInfo = [&](const auto stage) {
        const int id = getStageIndex(stage);
        if(moduleInfo_[id].shaderModule == VK_NULL_HANDLE)
        {
            return;
        }
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
        stageCreateInfo.pSpecializationInfo
            = specSizes.size() > 0 ? &specInfoList_[index] : nullptr;

        stageCreateInfoList_.emplace_back(stageCreateInfo);

        if(specSizes.size() > 0)
        {
            index++;
        }
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

    colorBlendStateInfo_.attachmentCount
        = static_cast<uint32_t>(colorBlendAttachmentStates_.size());
    colorBlendStateInfo_.pAttachments = colorBlendAttachmentStates_.data();

    // Dynamic states
    dynamicStateInfo_.dynamicStateCount = static_cast<uint32_t>(dynamicStates_.size());
    dynamicStateInfo_.pDynamicStates = dynamicStates_.data();
}
} // namespace vkw