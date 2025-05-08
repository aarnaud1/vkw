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

#include "vkw/wrappers/ComputePipeline.hpp"

#include <stdexcept>

namespace vkw
{
ComputePipeline::ComputePipeline(Device& device, const std::string& shaderSource)
{
    VKW_CHECK_BOOL_THROW(this->init(device, shaderSource), "Initializing compute pipeline");
}

ComputePipeline::ComputePipeline(ComputePipeline&& cp) { *this = std::move(cp); }

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(shaderSource_, cp.shaderSource_);
    std::swap(pipeline_, cp.pipeline_);
    std::swap(initialized_, cp.initialized_);

    return *this;
}

ComputePipeline::~ComputePipeline() { this->clear(); }

bool ComputePipeline::init(Device& device, const std::string& shaderSource)
{
    if(!initialized_)
    {
        device_ = &device;
        shaderSource_ = shaderSource;

        specData_.reserve(1024);
        specSizes_.reserve(32);

        initialized_ = true;
    }

    return true;
}

void ComputePipeline::clear()
{
    if(pipeline_ != VK_NULL_HANDLE)
    {
        device_->vk().vkDestroyPipeline(device_->getHandle(), pipeline_, nullptr);
    }

    device_ = nullptr;
    pipeline_ = VK_NULL_HANDLE;

    specData_.clear();
    specSizes_.clear();

    initialized_ = false;
}

void ComputePipeline::createPipeline(PipelineLayout& pipelineLayout)
{
    if(!initialized_)
    {
        throw std::runtime_error("Attempting to allocate an empty ComputePipeline object");
    }

    const auto src = utils::readShader(shaderSource_);
    auto shaderModule = utils::createShaderModule(device_->vk(), device_->getHandle(), src);

    size_t offset = 0;
    std::vector<VkSpecializationMapEntry> specMap;
    for(size_t i = 0; i < specSizes_.size(); i++)
    {
        VkSpecializationMapEntry mapEntry
            = {static_cast<uint32_t>(i), static_cast<uint32_t>(offset), specSizes_[i]};
        specMap.push_back(mapEntry);
        offset += specSizes_[i];
    }

    VkSpecializationInfo specInfo{};
    specInfo.mapEntryCount = static_cast<uint32_t>(specMap.size());
    specInfo.pMapEntries = specMap.data();
    specInfo.pData = specData_.data();
    specInfo.dataSize = specData_.size();

    VkPipelineShaderStageCreateInfo stageCreateInfo{};
    stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageCreateInfo.pNext = nullptr;
    stageCreateInfo.flags = 0;
    stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageCreateInfo.module = shaderModule;
    stageCreateInfo.pName = "main";
    stageCreateInfo.pSpecializationInfo = (specSizes_.size() > 0) ? &specInfo : nullptr;

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stage = stageCreateInfo;
    createInfo.layout = pipelineLayout.getHandle();
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;

    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateComputePipelines(
            device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_),
        "Creating compute pipeline");

    device_->vk().vkDestroyShaderModule(device_->getHandle(), shaderModule, nullptr);
}
} // namespace vkw
