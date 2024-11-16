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

#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
PipelineLayout::PipelineLayout(Device &device, const size_t numSets)
{
    VKW_CHECK_BOOL_THROW(this->init(device, numSets), "Initializing pipeline layout");
}

PipelineLayout::PipelineLayout(PipelineLayout &&cp) { *this = std::move(cp); }

PipelineLayout &PipelineLayout::operator=(PipelineLayout &&cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(layout_, cp.layout_);

    std::swap(setLayouts_, cp.setLayouts_);

    std::swap(offset_, cp.offset_);
    std::swap(pushConstantRanges_, cp.pushConstantRanges_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

PipelineLayout::~PipelineLayout() { this->clear(); }

bool PipelineLayout::init(Device &device, const size_t numSets)
{
    if(!initialized_)
    {
        device_ = &device;

        setLayouts_.resize(numSets);
        for(auto &setLayout : setLayouts_)
        {
            setLayout.init(*device_);
        }

        initialized_ = true;
    }

    return true;
}

void PipelineLayout::clear()
{
    VKW_DELETE_VK(PipelineLayout, layout_);
    for(auto &setLayout : setLayouts_)
    {
        setLayout.clear();
    }
    setLayouts_.clear();

    offset_ = 0;
    pushConstantRanges_.clear();

    device_ = nullptr;
    initialized_ = false;
}

void PipelineLayout::create()
{
    createDescriptorSetLayouts();

    std::vector<VkDescriptorSetLayout> layouts;
    for(auto &layout : setLayouts_)
    {
        layouts.push_back(layout.getHandle());
    }

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts_.size());
    createInfo.pSetLayouts = layouts.data();
    createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges_.size());
    createInfo.pPushConstantRanges
        = (pushConstantRanges_.size() > 0) ? pushConstantRanges_.data() : nullptr;

    VKW_CHECK_VK_THROW(
        vkCreatePipelineLayout(device_->getHandle(), &createInfo, nullptr, &layout_),
        "Creating pipeline layout");
}

void PipelineLayout::createDescriptorSetLayouts()
{
    for(size_t i = 0; i < setLayouts_.size(); i++)
    {
        setLayouts_[i].create();
    }
}
} // namespace vkw
