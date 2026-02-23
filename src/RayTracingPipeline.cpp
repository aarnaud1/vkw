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

#include "vkw/detail/RayTracingPipeline.hpp"

namespace vkw
{
RayTracingPipeline::RayTracingPipeline(const Device& device)
{
    VKW_CHECK_BOOL_FAIL(this->init(device), "Creating raytracing pipeline");
}

RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&& rhs) { *this = std::move(rhs); }

RayTracingPipeline& RayTracingPipeline::operator=(RayTracingPipeline&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(pipeline_, rhs.pipeline_);

    std::swap(dynamicStates_, rhs.dynamicStates_);
    std::swap(dynamicStateInfo_, rhs.dynamicStateInfo_);

    std::swap(initialized_, rhs.initialized_);

    std::swap(moduleInfo_, rhs.moduleInfo_);
    std::swap(specMaps_, rhs.specMaps_);
    std::swap(specInfo_, rhs.specInfo_);
    std::swap(shaderStages_, rhs.shaderStages_);
    std::swap(shaderGroups_, rhs.shaderGroups_);

    return *this;
}

bool RayTracingPipeline::init(const Device& device)
{
    device_ = &device;

    // Dynamic states
    dynamicStateInfo_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo_.dynamicStateCount = 0;
    dynamicStateInfo_.pDynamicStates = nullptr;

    initialized_ = true;

    return true;
}

RayTracingPipeline::~RayTracingPipeline() { clear(); }

void RayTracingPipeline::clear()
{
    clearShaderModules();
    VKW_DELETE_VK(Pipeline, pipeline_);

    device_ = nullptr;
    initialized_ = false;
}

bool RayTracingPipeline::addShaderStage(
    const VkShaderStageFlagBits stage, const std::string& shaderSource, const char* pName)
{
    VKW_ASSERT(this->initialized());

    moduleInfo_.emplace_back();

    auto& moduleInfo = moduleInfo_.back();

    const auto pSource = utils::readShader(shaderSource);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = pSource.size() * sizeof(decltype(pSource)::value_type);
    createInfo.pCode = reinterpret_cast<const uint32_t*>(pSource.data());
    VKW_CHECK_BOOL_RETURN_FALSE(device_->vk().vkCreateShaderModule(
        device_->getHandle(), &createInfo, nullptr, &moduleInfo.shaderModule));

    moduleInfo.shaderStage = stage;
    moduleInfo.pName = std::string(pName);

    return true;
}

bool RayTracingPipeline::addShaderStage(
    const VkShaderStageFlagBits stage, const char* srcData, const size_t byteCount, const char* pName)
{
    VKW_ASSERT(this->initialized());

    moduleInfo_.emplace_back();
    specMaps_.emplace_back();

    auto& moduleInfo = moduleInfo_.back();

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = byteCount;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(srcData);
    VKW_CHECK_BOOL_RETURN_FALSE(device_->vk().vkCreateShaderModule(
        device_->getHandle(), &createInfo, nullptr, &moduleInfo.shaderModule));

    moduleInfo.shaderStage = stage;
    moduleInfo.pName = std::string(pName);

    return true;
}

RayTracingPipeline& RayTracingPipeline::addGeneralShaderGroup(const uint32_t shaderIndex)
{
    VKW_ASSERT(this->initialized());

    VkRayTracingShaderGroupCreateInfoKHR shaderGroupCreateInfo = {};
    shaderGroupCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroupCreateInfo.pNext = nullptr;
    shaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shaderGroupCreateInfo.generalShader = shaderIndex;
    shaderGroupCreateInfo.closestHitShader = 0;
    shaderGroupCreateInfo.anyHitShader = 0;
    shaderGroupCreateInfo.intersectionShader = 0;
    shaderGroupCreateInfo.pShaderGroupCaptureReplayHandle = nullptr;
    shaderGroups_.emplace_back(shaderGroupCreateInfo);

    return *this;
}

RayTracingPipeline& RayTracingPipeline::addTriangleHitShaderGroup(
    const uint32_t closestHitIndex, const uint32_t anyHitIndex, const uint32_t intersectionIndex)
{
    VKW_ASSERT(this->initialized());

    VkRayTracingShaderGroupCreateInfoKHR shaderGroupCreateInfo = {};
    shaderGroupCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroupCreateInfo.pNext = nullptr;
    shaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    shaderGroupCreateInfo.generalShader = 0;
    shaderGroupCreateInfo.closestHitShader = closestHitIndex;
    shaderGroupCreateInfo.anyHitShader = anyHitIndex;
    shaderGroupCreateInfo.intersectionShader = intersectionIndex;
    shaderGroupCreateInfo.pShaderGroupCaptureReplayHandle = nullptr;
    shaderGroups_.emplace_back(shaderGroupCreateInfo);

    return *this;
}

RayTracingPipeline& RayTracingPipeline::addProceduralHitShaderGroup(
    const uint32_t closestHitIndex, const uint32_t anyHitIndex, const uint32_t intersectionIndex)
{
    VKW_ASSERT(this->initialized());

    VkRayTracingShaderGroupCreateInfoKHR shaderGroupCreateInfo = {};
    shaderGroupCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroupCreateInfo.pNext = nullptr;
    shaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
    shaderGroupCreateInfo.generalShader = 0;
    shaderGroupCreateInfo.closestHitShader = closestHitIndex;
    shaderGroupCreateInfo.anyHitShader = anyHitIndex;
    shaderGroupCreateInfo.intersectionShader = intersectionIndex;
    shaderGroupCreateInfo.pShaderGroupCaptureReplayHandle = nullptr;
    shaderGroups_.emplace_back(shaderGroupCreateInfo);

    return *this;
}

bool RayTracingPipeline::createPipeline(
    const PipelineLayout& pipelineLayout, const uint32_t maxDepth, const VkPipelineCreateFlags flags)
{
    VKW_ASSERT(this->initialized());

    finalizePipelineStages();

    VkRayTracingPipelineCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    createInfo.stageCount = static_cast<uint32_t>(shaderStages_.size());
    createInfo.pStages = shaderStages_.data();
    createInfo.groupCount = static_cast<uint32_t>(shaderGroups_.size());
    createInfo.pGroups = shaderGroups_.data();
    createInfo.maxPipelineRayRecursionDepth = maxDepth;
    /// @todo: Add pipeline libraries support
    createInfo.pLibraryInfo = nullptr;
    /// @todo: Add library interface support
    createInfo.pLibraryInterface = nullptr;
    createInfo.pDynamicState = &dynamicStateInfo_;
    createInfo.layout = pipelineLayout.getHandle();
    /// @todo: Add support for pipeline derivatives
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = -1;
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateRayTracingPipelinesKHR(
        device_->getHandle(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_));

    clearShaderModules();

    return true;
}

void RayTracingPipeline::finalizePipelineStages()
{
    VKW_ASSERT(this->initialized());

    const size_t stageCount = moduleInfo_.size();
    specMaps_.resize(stageCount);
    specInfo_.resize(stageCount);
    shaderStages_.resize(stageCount);

    for(size_t id = 0; id < stageCount; ++id)
    {
        size_t offset = 0;
        const auto& moduleInfo = moduleInfo_[id];
        auto& specMap = specMaps_[id];
        auto& specInfo = specInfo_[id];
        auto& shaderStage = shaderStages_[id];

        for(size_t i = 0; i < moduleInfo.specSizes.size(); i++)
        {
            VkSpecializationMapEntry mapEntry
                = {static_cast<uint32_t>(i), static_cast<uint32_t>(offset), moduleInfo.specSizes[i]};
            specMap.push_back(mapEntry);
            offset += moduleInfo.specSizes[i];
        }

        if(moduleInfo.specSizes.size() != 0)
        {
            specInfo.mapEntryCount = static_cast<uint32_t>(specMap.size());
            specInfo.pMapEntries = specMap.data();
            specInfo.dataSize = moduleInfo_[id].specData.size();
            specInfo.pData = moduleInfo_[id].specData.data();
        }

        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.pName = nullptr;
        shaderStage.flags = 0;
        shaderStage.stage = moduleInfo.shaderStage;
        shaderStage.module = moduleInfo.shaderModule;
        shaderStage.pName = "main";
        shaderStage.pSpecializationInfo = moduleInfo.specSizes.size() > 0 ? &specInfo : nullptr;
    }

    // Dynamic states
    dynamicStateInfo_.dynamicStateCount = static_cast<uint32_t>(dynamicStates_.size());
    dynamicStateInfo_.pDynamicStates = dynamicStates_.data();
}

void RayTracingPipeline::clearShaderModules()
{
    VKW_ASSERT(device_ != nullptr);

    for(const auto& moduleInfo : moduleInfo_)
    {
        if(moduleInfo.shaderModule != VK_NULL_HANDLE)
        {
            device_->vk().vkDestroyShaderModule(device_->getHandle(), moduleInfo.shaderModule, nullptr);
        }
    }

    moduleInfo_.clear();
    specMaps_.clear();
    specInfo_.clear();
    shaderStages_.clear();
}
} // namespace vkw
