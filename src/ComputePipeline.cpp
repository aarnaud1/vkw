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

#include "vkw/detail/ComputePipeline.hpp"

#include <stdexcept>

namespace vkw
{
ComputePipeline::ComputePipeline(const Device& device, const std::string& shaderSource)
{
    VKW_CHECK_BOOL_FAIL(this->init(device, shaderSource), "Initializing compute pipeline");
}

ComputePipeline::ComputePipeline(const Device& device, const char* shaderSource, const size_t byteCount)
{
    VKW_CHECK_BOOL_FAIL(this->init(device, shaderSource, byteCount), "Initializing compute pipeline");
}

ComputePipeline::ComputePipeline(ComputePipeline&& cp) { *this = std::move(cp); }

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(shaderSource_, cp.shaderSource_);
    std::swap(shaderSourceBytes_, cp.shaderSourceBytes_);
    std::swap(pipeline_, cp.pipeline_);
    std::swap(initialized_, cp.initialized_);

    return *this;
}

ComputePipeline::~ComputePipeline() { this->clear(); }

bool ComputePipeline::init(const Device& device, const std::string& shaderSource)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    shaderSource_ = shaderSource;

    specData_.reserve(1024);
    specSizes_.reserve(32);

    initialized_ = true;

    return true;
}

bool ComputePipeline::init(const Device& device, const char* shaderSource, const size_t byteCount)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    shaderSource_ = "";
    shaderSourceBytes_.resize(byteCount);
    memcpy(shaderSourceBytes_.data(), shaderSource, byteCount);

    specData_.reserve(1024);
    specSizes_.reserve(32);

    initialized_ = true;

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

bool ComputePipeline::createPipeline(PipelineLayout& pipelineLayout)
{
    VKW_ASSERT(this->initialized());

    auto shaderModule = utils::createShaderModule(
        device_->vk(), device_->getHandle(),
        shaderSourceBytes_.empty() ? utils::readShader(shaderSource_) : shaderSourceBytes_);
    shaderSourceBytes_.clear();

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

    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateComputePipelines(
        device_->getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline_));

    device_->vk().vkDestroyShaderModule(device_->getHandle(), shaderModule, nullptr);

    return true;
}
} // namespace vkw
