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

#include <stdexcept>

namespace vkw
{
DescriptorPool::DescriptorPool(
    Device &device, PipelineLayout &pipelineLayout, VkShaderStageFlags flags)
{
    this->init(device, pipelineLayout, flags);
}

DescriptorPool::~DescriptorPool() { this->clear(); }

void DescriptorPool::init(
    Device &device, PipelineLayout &pipelineLayout, VkShaderStageFlags /*flags*/)
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

    auto &descriptorSetLayouts = pipelineLayout.getDescriptorSetLayouts();
    for(size_t i = 0; i < descriptorSetLayouts.size(); i++)
    {
        nStorageBufferBindings
            += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumStorageBufferBindings();
        nUniformBufferBindings
            += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumUniformBufferBindings();
        nStorageImageBindings
            += pipelineLayout.getDescriptorSetlayoutInfo(i).getNumStorageImageBindings();
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
} // namespace vkw
