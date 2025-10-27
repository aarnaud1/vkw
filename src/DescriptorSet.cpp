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

#include "vkw/detail/DescriptorSet.hpp"

namespace vkw
{
DescriptorSet::DescriptorSet(
    const Device& device, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool,
    const void* pCreateNext)
{
    VKW_CHECK_BOOL_FAIL(
        this->init(device, layout, descriptorPool, pCreateNext), "Error initializing descriptor set");
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs)
{
    this->clear();

    std::swap(device_, rhs.device_);
    std::swap(descriptorPool_, rhs.descriptorPool_);
    std::swap(descriptorSet_, rhs.descriptorSet_);
    std::swap(initialized_, rhs.initialized_);

    return *this;
}

DescriptorSet::~DescriptorSet() { this->clear(); }

bool DescriptorSet::init(
    const Device& device, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool,
    const void* pCreateNext)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    descriptorPool_ = &descriptorPool;

    auto layoutHandle = layout.getHandle();

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = pCreateNext;
    allocateInfo.descriptorPool = descriptorPool_->getHandle();
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &layoutHandle;
    VKW_INIT_CHECK_VK(
        device_->vk().vkAllocateDescriptorSets(device_->getHandle(), &allocateInfo, &descriptorSet_));

    initialized_ = true;
    return true;
}

void DescriptorSet::clear()
{
    if(descriptorSet_ != VK_NULL_HANDLE)
    {
        device_->vk().vkFreeDescriptorSets(
            device_->getHandle(), descriptorPool_->getHandle(), 1, &descriptorSet_);
        descriptorSet_ = VK_NULL_HANDLE;
    }

    descriptorPool_ = nullptr;
    device_ = nullptr;
    initialized_ = false;
}

DescriptorSet& DescriptorSet::bindSampler(const uint32_t binding, const VkSampler sampler)
{
    const VkDescriptorImageInfo imgInfo = {sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindCombinedImageSampler(
    const uint32_t binding, const VkSampler sampler, const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {sampler, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindSampledImage(
    const uint32_t binding, const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindStorageImage(
    const uint32_t binding, const VkImageView imageView, const VkImageLayout layout)
{
    return bindStorageImageIndex(binding, imageView, 0, layout);
}

DescriptorSet& DescriptorSet::bindStorageImageIndex(
    const uint32_t binding, const VkImageView imageView, const uint32_t index, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformTexelBuffer(const uint32_t binding, const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindStorageTexelBuffer(const uint32_t binding, const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindStorageBuffer(
    const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range)
{
    return bindStorageBufferIndex(binding, buffer, 0, offset, range);
}

DescriptorSet& DescriptorSet::bindStorageBufferIndex(
    const uint32_t binding, const VkBuffer buffer, const uint32_t index, const VkDeviceSize offset,
    const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformBuffer(
    const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindStorageBufferDynamic(
    const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformBufferDynamic(
    const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindAccelerationStructure(
    const uint32_t binding, const VkAccelerationStructureKHR accelerationStructure)
{
    VkWriteDescriptorSetAccelerationStructureKHR asWriteDescriptorSet = {};
    asWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    asWriteDescriptorSet.pNext = nullptr;
    asWriteDescriptorSet.accelerationStructureCount = 1;
    asWriteDescriptorSet.pAccelerationStructures = &accelerationStructure;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = &asWriteDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}
} // namespace vkw