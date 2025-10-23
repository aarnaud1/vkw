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

#include "vkw/detail/DescriptorSetLayout.hpp"

#include "vkw/detail/utils.hpp"

namespace vkw
{
DescriptorSetLayout::DescriptorSetLayout(const Device& device)
{
    VKW_CHECK_BOOL_FAIL(this->init(device), "Initializing DescriptorSetLayout");
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

bool DescriptorSetLayout::init(const Device& device)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    descriptorSetLayout_ = VK_NULL_HANDLE;
    initialized_ = true;

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

bool DescriptorSetLayout::create(const void* pCreateNext) { return this->create({}, pCreateNext); }

bool DescriptorSetLayout::create(const VkDescriptorSetLayoutCreateFlags flags, const void* pCreateNext)
{
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = pCreateNext;
    createInfo.flags = flags;
    createInfo.bindingCount = static_cast<uint32_t>(bindings_.size());
    createInfo.pBindings = reinterpret_cast<const VkDescriptorSetLayoutBinding*>(bindings_.data());

    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateDescriptorSetLayout(
        device_->getHandle(), &createInfo, nullptr, &descriptorSetLayout_));

    return true;
}
} // namespace vkw
