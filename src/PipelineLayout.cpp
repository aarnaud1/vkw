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

bool PipelineLayout::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;

        initialized_ = true;
    }
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
    createInfo.pPushConstantRanges
        = (pushConstantRanges.size() > 0) ? pushConstantRanges.data() : nullptr;

    VKW_CHECK_VK_FAIL(
        device_->vk().vkCreatePipelineLayout(
            device_->getHandle(), &createInfo, nullptr, &pipelineLayout_),
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
