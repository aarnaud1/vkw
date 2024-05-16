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

#include "vkWrappers/wrappers/PipelineLayout.hpp"

namespace vk
{
PipelineLayout::PipelineLayout(Device &device, size_t numSets)
    : device_(&device), setLayoutInfo_(numSets), setLayouts_(numSets)
{}

PipelineLayout::~PipelineLayout()
{
    if(layout_ != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device_->getHandle(), layout_, nullptr);

        for(auto setLayout : setLayouts_)
        {
            vkDestroyDescriptorSetLayout(device_->getHandle(), setLayout, nullptr);
        }
    }
}

void PipelineLayout::create()
{
    createDescriptorSetLayouts();

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = setLayouts_.size();
    createInfo.pSetLayouts = setLayouts_.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges_.size());
    createInfo.pPushConstantRanges
        = pushConstantRanges_.size() > 0 ? pushConstantRanges_.data() : nullptr;

    CHECK_VK(
        vkCreatePipelineLayout(device_->getHandle(), &createInfo, nullptr, &layout_),
        "Creating pipeline layout");
}

void PipelineLayout::createDescriptorSetLayouts()
{
    for(size_t i = 0; i < setLayoutInfo_.size(); i++)
    {
        auto &bindings = setLayoutInfo_[i].getBindings();
        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        createInfo.pBindings
            = reinterpret_cast<const VkDescriptorSetLayoutBinding *>(bindings.data());

        CHECK_VK(
            vkCreateDescriptorSetLayout(
                device_->getHandle(), &createInfo, nullptr, &setLayouts_[i]),
            "Creating descriptor set layout");
    }
}
} // namespace vk
