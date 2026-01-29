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
bool RayTracingPipeline::init(const Device& device)
{
    device_ = &device;

    return true;
}

void RayTracingPipeline::clear() { clearShaderModules(); }

bool RayTracingPipeline::addShaderStage(
    const VkShaderStageFlags stage, const std::string& shaderSource, const char* pName)
{
    VKW_ASSERT(this->initialized());

    moduleInfo_.emplace_back();
    specMaps_.emplace_back();

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
    const VkShaderStageFlags stage, const char* srcData, const size_t byteCount, const char* pName)
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
    return *this;
}

RayTracingPipeline& RayTracingPipeline::addHitShaderGroup(
    const uint32_t closestHitIndex, const uint32_t anyHitIndex, const uint32_t intersectionIndex)
{
    VKW_ASSERT(this->initialized());
    return *this;
}

bool RayTracingPipeline::createPipeline(
    const PipelineLayout& pipelineLayout, const uint32_t maxDepth, const VkPipelineCreateFlags flags)
{
    VKW_ASSERT(this->initialized());

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

    return true;
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
}
} // namespace vkw
