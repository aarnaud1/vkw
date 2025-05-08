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

#include "vkw/wrappers/DescriptorSet.hpp"

namespace vkw
{
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
    uint32_t binding,
    const VkSampler sampler,
    const VkImageView imageView,
    const VkImageLayout layout)
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
    uint32_t binding, VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformTexelBuffer(
    const uint32_t binding, const VkBufferView& bufferView)
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

DescriptorSet& DescriptorSet::bindStorageTexelBuffer(
    const uint32_t binding, const VkBufferView& bufferView)
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
    const uint32_t binding,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformBuffer(
    const uint32_t binding,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize range)
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
    const uint32_t binding,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize range)
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
    const uint32_t binding,
    const VkBuffer buffer,
    const VkDeviceSize offset,
    const VkDeviceSize range)
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