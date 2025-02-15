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

#include "vkWrappers/wrappers/DescriptorPool.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#include <stdexcept>

namespace vkw
{
DescriptorPool::DescriptorPool(
    Device& device, const uint32_t maxSetCount, const uint32_t maxPoolSize)
{
    VKW_CHECK_BOOL_THROW(
        this->init(device, maxSetCount, maxPoolSize), "Creating descriptor pool\n");
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);

    std::swap(descriptorSets_, cp.descriptorSets_);
    std::swap(descriptorPool_, cp.descriptorPool_);

    std::swap(maxSetCount_, cp.maxSetCount_);
    std::swap(maxPoolSize_, cp.maxPoolSize_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

bool DescriptorPool::init(Device& device, const uint32_t maxSetCount, const uint32_t maxPoolSize)
{
    if(!initialized_)
    {
        device_ = &device;

        maxSetCount_ = maxSetCount;
        maxPoolSize_ = maxPoolSize;

        descriptorSets_.reserve(maxSetCount);

        std::vector<VkDescriptorPoolSize> poolSizes{};
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxPoolSize_});
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxPoolSize_});
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxPoolSize_});
        poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPoolSize_});

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
    }

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
        maxPoolSize_ = 0;

        device_ = nullptr;
        initialized_ = false;
    }
}

std::vector<DescriptorSet> DescriptorPool::allocateDescriptorSets(
    const DescriptorSetLayout& layout, const uint32_t count)
{
    std::vector<VkDescriptorSetLayout> descriptorSetlayouts{};
    descriptorSetlayouts.resize(count);
    std::fill(descriptorSetlayouts.begin(), descriptorSetlayouts.end(), layout.getHandle());

    std::vector<VkDescriptorSet> descriptorSets{};
    descriptorSets.resize(count);

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = descriptorPool_;
    allocateInfo.descriptorSetCount = count;
    allocateInfo.pSetLayouts = descriptorSetlayouts.data();
    VKW_CHECK_VK_THROW(
        device_->vk().vkAllocateDescriptorSets(
            device_->getHandle(), &allocateInfo, descriptorSets.data()),
        "Allocating descriptor sets");

    descriptorSets_.insert(descriptorSets_.end(), descriptorSets.begin(), descriptorSets.end());

    std::vector<DescriptorSet> ret;
    ret.resize(count);
    for(uint32_t i = 0; i < count; ++i)
    {
        ret[i].device_ = device_;
        ret[i].descriptorSet_ = descriptorSets[i];
    }
    return ret;
}

DescriptorSet DescriptorPool::allocateDescriptorSet(const DescriptorSetLayout& layout)
{
    VkDescriptorSetLayout descriptorSetLayout = layout.getHandle();
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = descriptorPool_;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &descriptorSetLayout;
    VKW_CHECK_VK_THROW(
        device_->vk().vkAllocateDescriptorSets(device_->getHandle(), &allocateInfo, &descriptorSet),
        "Allocating descriptor set");

    descriptorSets_.push_back(descriptorSet);

    DescriptorSet ret{};
    ret.device_ = device_;
    ret.descriptorSet_ = descriptorSet;
    return ret;
}
} // namespace vkw