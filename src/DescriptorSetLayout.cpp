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

#include "vkw/wrappers/DescriptorSetLayout.hpp"

#include "vkw/wrappers/utils.hpp"

namespace vkw
{
DescriptorSetLayout::DescriptorSetLayout(Device& device)
{
    VKW_CHECK_BOOL_THROW(this->init(device), "Initializing DescriptorSetLayout");
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(descriptorSetLayout_, cp.descriptorSetLayout_);

    std::swap(bindings_, cp.bindings_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

bool DescriptorSetLayout::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;
        descriptorSetLayout_ = VK_NULL_HANDLE;

        initialized_ = true;
    }

    return true;
}

void DescriptorSetLayout::clear()
{
    bindings_.clear();

    VKW_DELETE_VK(DescriptorSetLayout, descriptorSetLayout_);
    memset(bindingCounts_, 0, descriptorTypeCount * sizeof(uint32_t));

    device_ = nullptr;
    initialized_ = false;
}

void DescriptorSetLayout::create()
{
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = static_cast<uint32_t>(bindings_.size());
    createInfo.pBindings = reinterpret_cast<const VkDescriptorSetLayoutBinding*>(bindings_.data());

    VKW_CHECK_VK_THROW(
        device_->vk().vkCreateDescriptorSetLayout(
            device_->getHandle(), &createInfo, nullptr, &descriptorSetLayout_),
        "Creating descriptor set layout");
}
} // namespace vkw
