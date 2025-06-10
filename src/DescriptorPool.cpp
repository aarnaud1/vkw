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

#include "vkw/detail/DescriptorPool.hpp"

#include "vkw/detail/utils.hpp"

#include <stdexcept>

namespace vkw
{
DescriptorPool::DescriptorPool(
    const Device& device,
    const uint32_t maxSetCount,
    const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VKW_CHECK_BOOL_FAIL(this->init(device, maxSetCount, poolSizes), "Creating descriptor pool\n");
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);

    std::swap(descriptorSets_, cp.descriptorSets_);
    std::swap(descriptorPool_, cp.descriptorPool_);

    std::swap(maxSetCount_, cp.maxSetCount_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

bool DescriptorPool::init(
    const Device& device,
    const uint32_t maxSetCount,
    const std::vector<VkDescriptorPoolSize>& poolSizes)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;

    maxSetCount_ = maxSetCount;

    descriptorSets_.reserve(maxSetCount);

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = maxSetCount_;
    createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateDescriptorPool(
        device_->getHandle(), &createInfo, nullptr, &descriptorPool_));

    initialized_ = true;

    return true;
}

void DescriptorPool::clear()
{
    if(initialized_)
    {
        if(descriptorPool_ != VK_NULL_HANDLE)
        {
            if(!descriptorSets_.empty())
            {
                device_->vk().vkFreeDescriptorSets(
                    device_->getHandle(),
                    descriptorPool_,
                    static_cast<uint32_t>(descriptorSets_.size()),
                    descriptorSets_.data());
                descriptorSets_.clear();
            }

            device_->vk().vkDestroyDescriptorPool(device_->getHandle(), descriptorPool_, nullptr);
            descriptorPool_ = VK_NULL_HANDLE;
        }

        maxSetCount_ = 0;

        device_ = nullptr;
        initialized_ = false;
    }
}
} // namespace vkw