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

#include "vkw/detail/PipelineLayout.hpp"

#include "vkw/detail/utils.hpp"

namespace vkw
{
PipelineLayout::PipelineLayout(PipelineLayout&& cp) { *this = std::move(cp); }

PipelineLayout& PipelineLayout::operator=(PipelineLayout&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);

    std::swap(descriptorSetLayouts_, cp.descriptorSetLayouts_);
    std::swap(pipelineLayout_, cp.pipelineLayout_);

    std::swap(offset_, cp.offset_);
    std::swap(ranges_, cp.ranges_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

bool PipelineLayout::init(const Device& device)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    initialized_ = true;

    return true;
}

void PipelineLayout::create()
{
    std::vector<VkPushConstantRange> pushConstantRanges{};
    for(const auto& range : ranges_)
    {
        if(range.size != 0)
        {
            pushConstantRanges.push_back(range);
        }
    }

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts_.size());
    createInfo.pSetLayouts = descriptorSetLayouts_.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
    createInfo.pPushConstantRanges = (pushConstantRanges.size() > 0) ? pushConstantRanges.data() : nullptr;

    VKW_CHECK_VK_FAIL(
        device_->vk().vkCreatePipelineLayout(device_->getHandle(), &createInfo, nullptr, &pipelineLayout_),
        "Creating pipeline layout");
}

void PipelineLayout::clear()
{
    VKW_DELETE_VK(PipelineLayout, pipelineLayout_);
    descriptorSetLayouts_.clear();

    offset_ = 0;
    memset(ranges_, 0, shaderStageCount * sizeof(VkPushConstantRange));
    device_ = nullptr;
    initialized_ = false;
}
} // namespace vkw
