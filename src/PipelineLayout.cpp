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
PipelineLayout::PipelineLayout(Device &device, const size_t numSets)
{
    this->init(device, numSets);
}

PipelineLayout::PipelineLayout(PipelineLayout &&cp) { *this = std::move(cp); }

PipelineLayout &PipelineLayout::operator=(PipelineLayout &&cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(layout_, cp.layout_);

    std::swap(setLayoutInfo_, cp.setLayoutInfo_);
    std::swap(setLayouts_, cp.setLayouts_);

    std::swap(offset_, cp.offset_);
    std::swap(pushConstantRanges_, cp.pushConstantRanges_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

PipelineLayout::~PipelineLayout() { this->clear(); }

void PipelineLayout::init(Device &device, const size_t numSets)
{
    if(!initialized_)
    {
        device_ = &device;

        setLayoutInfo_.resize(numSets);
        setLayouts_.resize(numSets);

        initialized_ = true;
    }
}

void PipelineLayout::clear()
{
    if(layout_ != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device_->getHandle(), layout_, nullptr);

        for(auto setLayout : setLayouts_)
        {
            vkDestroyDescriptorSetLayout(device_->getHandle(), setLayout, nullptr);
        }
    }

    device_ = nullptr;
    layout_ = VK_NULL_HANDLE;

    setLayoutInfo_.clear();
    setLayouts_.clear();

    offset_ = 0;
    pushConstantRanges_.clear();

    initialized_ = false;
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
