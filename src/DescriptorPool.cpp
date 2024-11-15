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

#include "vkWrappers/wrappers/DescriptorPool.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#include <stdexcept>

namespace vkw
{
DescriptorPool::DescriptorPool(
    Device& device, const uint32_t maxSetCount, const uint32_t maxPoolSize)
{
    CHECK_BOOL_THROW(this->init(device, maxSetCount, maxPoolSize), "Creating descriptor pool\n");
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
        createInfo.poolSizeCount = poolSizes.size();
        createInfo.pPoolSizes = poolSizes.data();
        CHECK_VK_RETURN_FALSE(
            vkCreateDescriptorPool(device_->getHandle(), &createInfo, nullptr, &descriptorPool_));

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
                vkFreeDescriptorSets(
                    device_->getHandle(),
                    descriptorPool_,
                    descriptorSets_.size(),
                    descriptorSets_.data());
                descriptorSets_.clear();
            }

            vkDestroyDescriptorPool(device_->getHandle(), descriptorPool_, nullptr);
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
    CHECK_VK(
        vkAllocateDescriptorSets(device_->getHandle(), &allocateInfo, descriptorSets.data()),
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
    CHECK_VK(
        vkAllocateDescriptorSets(device_->getHandle(), &allocateInfo, &descriptorSet),
        "Allocating descriptor set");

    descriptorSets_.push_back(descriptorSet);

    DescriptorSet ret{};
    ret.device_ = device_;
    ret.descriptorSet_ = descriptorSet;
    return ret;
}
} // namespace vkw

/*
namespace vkw
{
DescriptorPool::DescriptorPool(Device &device, PipelineLayout &pipelineLayout)
{
this->init(device, pipelineLayout);
}

DescriptorPool::~DescriptorPool() { this->clear(); }

void DescriptorPool::init(Device &device, PipelineLayout &pipelineLayout)
{
if(!initialized_)
{
    device_ = &device;
    descriptorSets_.resize(pipelineLayout.numSets());

    allocateDescriptorSets(pipelineLayout);
    initialized_ = true;
}
}

void DescriptorPool::clear()
{
if(descriptorPool_ != VK_NULL_HANDLE)
{
    vkFreeDescriptorSets(
        device_->getHandle(), descriptorPool_, descriptorSets_.size(), descriptorSets_.data());

    vkDestroyDescriptorPool(device_->getHandle(), descriptorPool_, nullptr);
}

device_ = nullptr;
descriptorSets_.clear();
descriptorPool_ = VK_NULL_HANDLE;

initialized_ = false;
}

void DescriptorPool::allocateDescriptorSets(PipelineLayout &pipelineLayout)
{
uint32_t nStorageBufferBindings = 0;
uint32_t nUniformBufferBindings = 0;
uint32_t nStorageImageBindings = 0;
uint32_t nSamplerImageBindings = 0;

auto &descriptorSetLayouts = pipelineLayout.getDescriptorSetLayouts();
for(size_t i = 0; i < descriptorSetLayouts.size(); i++)
{
    nStorageBufferBindings
        += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumStorageBufferBindings();
    nUniformBufferBindings
        += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumUniformBufferBindings();
    nStorageImageBindings
        += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumStorageImageBindings();
    nSamplerImageBindings
        += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumSamplerImageBindings();
}

std::vector<VkDescriptorPoolSize> poolSizes;
if(nStorageBufferBindings > 0)
{
    VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nStorageBufferBindings};
    poolSizes.push_back(poolSize);
}

if(nUniformBufferBindings > 0)
{
    VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nUniformBufferBindings};
    poolSizes.push_back(poolSize);
}

if(nStorageImageBindings > 0)
{
    VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, nStorageImageBindings};
    poolSizes.push_back(poolSize);
}

if(nSamplerImageBindings > 0)
{
    VkDescriptorPoolSize poolSize
        = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nSamplerImageBindings};
    poolSizes.push_back(poolSize);
}

VkDescriptorPoolCreateInfo poolCreateInfo = {};
poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
poolCreateInfo.pNext = nullptr;
poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
poolCreateInfo.maxSets = descriptorSetLayouts.size();
poolCreateInfo.poolSizeCount = poolSizes.size();
poolCreateInfo.pPoolSizes = poolSizes.data();
CHECK_VK(
    vkCreateDescriptorPool(device_->getHandle(), &poolCreateInfo, nullptr, &descriptorPool_),
    "Creating block pool");

VkDescriptorSetAllocateInfo allocateInfo = {};
allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
allocateInfo.pNext = nullptr;
allocateInfo.descriptorPool = descriptorPool_;
allocateInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size());
allocateInfo.pSetLayouts = descriptorSetLayouts.data();
CHECK_VK(
    vkAllocateDescriptorSets(device_->getHandle(), &allocateInfo, descriptorSets_.data()),
    "Allocating descriptor sets");
}

DescriptorPool &DescriptorPool::bindStorageBuffer(
uint32_t setId,
uint32_t bindingId,
VkDescriptorBufferInfo bufferInfo,
uint32_t offset,
uint32_t count)
{
VkWriteDescriptorSet writeDescriptorSet = {};
writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
writeDescriptorSet.dstSet = descriptorSets_[setId];
writeDescriptorSet.dstBinding = bindingId;
writeDescriptorSet.dstArrayElement = offset;
writeDescriptorSet.descriptorCount = count;
writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
writeDescriptorSet.pImageInfo = nullptr;
writeDescriptorSet.pBufferInfo = &bufferInfo;
writeDescriptorSet.pTexelBufferView = nullptr;

vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
return *this;
}

DescriptorPool &DescriptorPool::bindStorageImage(
uint32_t setId,
uint32_t bindingId,
VkDescriptorImageInfo imageInfo,
uint32_t offset,
uint32_t count)
{
VkWriteDescriptorSet writeDescriptorSet = {};
writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
writeDescriptorSet.dstSet = descriptorSets_[setId];
writeDescriptorSet.dstBinding = bindingId;
writeDescriptorSet.dstArrayElement = offset;
writeDescriptorSet.descriptorCount = count;
writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
writeDescriptorSet.pImageInfo = &imageInfo;
writeDescriptorSet.pBufferInfo = nullptr;
writeDescriptorSet.pTexelBufferView = nullptr;

vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
return *this;
}

DescriptorPool &DescriptorPool::bindUniformBuffer(
uint32_t setId,
uint32_t bindingId,
VkDescriptorBufferInfo bufferInfo,
uint32_t offset,
uint32_t count)
{
VkWriteDescriptorSet writeDescriptorSet = {};
writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
writeDescriptorSet.dstSet = descriptorSets_[setId];
writeDescriptorSet.dstBinding = bindingId;
writeDescriptorSet.dstArrayElement = offset;
writeDescriptorSet.descriptorCount = count;
writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
writeDescriptorSet.pImageInfo = nullptr;
writeDescriptorSet.pBufferInfo = &bufferInfo;
writeDescriptorSet.pTexelBufferView = nullptr;

vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
return *this;
}

DescriptorPool &DescriptorPool::bindSamplerImage(
uint32_t setId,
uint32_t bindingId,
VkDescriptorImageInfo imageInfo,
uint32_t offset,
uint32_t count)
{
VkWriteDescriptorSet writeDescriptorSet = {};
writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
writeDescriptorSet.dstSet = descriptorSets_[setId];
writeDescriptorSet.dstBinding = bindingId;
writeDescriptorSet.dstArrayElement = offset;
writeDescriptorSet.descriptorCount = count;
writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
writeDescriptorSet.pImageInfo = &imageInfo;
writeDescriptorSet.pBufferInfo = nullptr;
writeDescriptorSet.pTexelBufferView = nullptr;

vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
return *this;
}
} // namespace vkw
*/