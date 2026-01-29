/*
 * Copyright (c) 2026 Adrien ARNAUD
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

#include "vkw/detail/utils.hpp"

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

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindSampler(
    const uint32_t binding, const uint32_t index, const VkSampler sampler)
{
    const VkDescriptorImageInfo imgInfo = {sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindSamplers(
    const uint32_t binding, const uint32_t index,
    std::initializer_list<std::reference_wrapper<Sampler>>& samplers)
{
    auto samplerList = utils::ScopedAllocator::allocateArray<VkSampler>(samplers.size());

    size_t samplerIndex = 0;
    for(const auto& sampler : samplers)
    {
        samplerList[samplerIndex++] = sampler.get().getHandle();
    }
    return bindSamplers(binding, index, samplerList);
}

DescriptorSet& DescriptorSet::bindSamplers(
    const uint32_t binding, const uint32_t index, const std::span<VkSampler>& samplers)
{
    auto imgInfo = utils::ScopedAllocator::allocateArray<VkDescriptorImageInfo>(samplers.size());

    size_t samplerIndex = 0;
    for(const auto sampler : samplers)
    {
        imgInfo[samplerIndex++] = {sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(imgInfo.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.pImageInfo = imgInfo.data();
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindCombinedImageSampler(
    const uint32_t binding, const uint32_t index, const VkSampler sampler, const VkImageView imageView,
    const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {sampler, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindCombinedImageSamplers(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<Sampler>>& samplers,
    const std::initializer_list<std::reference_wrapper<ImageView>>& imageViews,
    const std::span<VkImageLayout>& layouts)
{
    VKW_ASSERT(samplers.size() == imageViews.size());

    auto samplerList = utils::ScopedAllocator::allocateArray<VkSampler>(samplers.size());
    auto imgViewList = utils::ScopedAllocator::allocateArray<VkImageView>(imageViews.size());

    size_t samplerIndex = 0;
    for(const auto& sampler : samplers)
    {
        samplerList[samplerIndex++] = sampler.get().getHandle();
    }

    size_t imgViewIndex = 0;
    for(const auto& imgView : imageViews)
    {
        imgViewList[imgViewIndex++] = imgView.get().getHandle();
    }

    return bindCombinedImageSamplers(
        binding, index, {samplerList.data(), samplerList.size()}, imgViewList, layouts);
}

DescriptorSet& DescriptorSet::bindCombinedImageSamplers(
    const uint32_t binding, const uint32_t index, const std::span<VkSampler>& samplers,
    const std::span<VkImageView>& imageViews, const std::span<VkImageLayout>& layouts)
{
    VKW_ASSERT(samplers.size() == imageViews.size());
    VKW_ASSERT(layouts.empty() || (samplers.size() == layouts.size()));

    auto imgInfo = utils::ScopedAllocator::allocateArray<VkDescriptorImageInfo>(samplers.size());

    for(size_t i = 0; i < samplers.size(); ++i)
    {
        imgInfo[i] = {samplers[i], imageViews[i], layouts.empty() ? VK_IMAGE_LAYOUT_GENERAL : layouts[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(imgInfo.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = imgInfo.data();
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindSampledImage(
    const uint32_t binding, const uint32_t index, const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindSampledImages(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<ImageView>>& imageViews,
    const std::span<VkImageLayout>& layouts)
{
    auto imgViewList = utils::ScopedAllocator::allocateArray<VkImageView>(imageViews.size());

    size_t imgIndex = 0;
    for(const auto& imgView : imageViews)
    {
        imgViewList[imgIndex++] = imgView.get().getHandle();
    }

    return bindSampledImages(binding, index, imgViewList, layouts);
}

DescriptorSet& DescriptorSet::bindSampledImages(
    const uint32_t binding, const uint32_t index, const std::span<VkImageView>& imageViews,
    const std::span<VkImageLayout>& layouts)
{
    VKW_ASSERT(layouts.empty() || (layouts.size() == imageViews.size()));

    auto imgInfo = utils::ScopedAllocator::allocateArray<VkDescriptorImageInfo>(imageViews.size());

    for(size_t i = 0; i < imageViews.size(); ++i)
    {
        imgInfo[i] = {VK_NULL_HANDLE, imageViews[i], layouts.empty() ? VK_IMAGE_LAYOUT_GENERAL : layouts[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(imgInfo.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.pImageInfo = imgInfo.data();
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindStorageImage(
    const uint32_t binding, const uint32_t index, const VkImageView imageView, const VkImageLayout layout)
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

DescriptorSet& DescriptorSet::bindStorageImages(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<ImageView>>& imageViews,
    const std::span<VkImageLayout>& layouts)
{
    auto imgViewList = utils::ScopedAllocator::allocateArray<VkImageView>(imageViews.size());

    size_t imgIndex = 0;
    for(const auto& imgView : imageViews)
    {
        imgViewList[imgIndex++] = imgView.get().getHandle();
    }

    return bindStorageImages(binding, index, imgViewList, layouts);
}

DescriptorSet& DescriptorSet::bindStorageImages(
    const uint32_t binding, const uint32_t index, const std::span<VkImageView>& imageViews,
    const std::span<VkImageLayout>& layouts)
{
    VKW_ASSERT(layouts.empty() || (layouts.size() == imageViews.size()));

    auto imgInfo = utils::ScopedAllocator::allocateArray<VkDescriptorImageInfo>(imageViews.size());
    for(size_t i = 0; i < imageViews.size(); ++i)
    {
        imgInfo[i] = {VK_NULL_HANDLE, imageViews[i], layouts.empty() ? VK_IMAGE_LAYOUT_GENERAL : layouts[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(imgInfo.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet.pImageInfo = imgInfo.data();
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindUniformTexelBuffer(
    const uint32_t binding, const uint32_t index, const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformTexelBuffers(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<BufferView>>& bufferViews)
{
    auto bufferViewList = utils::ScopedAllocator::allocateArray<VkBufferView>(bufferViews.size());

    size_t bufferIndex = 0;
    for(const auto& bufferView : bufferViews)
    {
        bufferViewList[bufferIndex++] = bufferView.get().getHandle();
    }

    return bindUniformTexelBuffers(binding, index, bufferViewList);
}

DescriptorSet& DescriptorSet::bindUniformTexelBuffers(
    const uint32_t binding, const uint32_t index, const std::span<VkBufferView>& bufferViews)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(bufferViews.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = bufferViews.data();

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindStorageTexelBuffer(
    const uint32_t binding, const uint32_t index, const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindStorageTexelBuffers(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<BufferView>>& bufferViews)
{
    auto bufferViewList = utils::ScopedAllocator::allocateArray<VkBufferView>(bufferViews.size());

    size_t bufferIndex = 0;
    for(const auto& bufferView : bufferViews)
    {
        bufferViewList[bufferIndex++] = bufferView.get().getHandle();
    }

    return bindStorageTexelBuffers(binding, index, bufferViewList);
}

DescriptorSet& DescriptorSet::bindStorageTexelBuffers(
    const uint32_t binding, const uint32_t index, const std::span<VkBufferView>& bufferViews)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(bufferViews.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = bufferViews.data();

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindUniformBuffer(
    const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset,
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
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformBuffers(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<BaseBuffer>>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    auto bufferList = utils::ScopedAllocator::allocateArray<VkBuffer>(buffers.size());

    size_t bufferIndex = 0;
    for(const auto& buffer : buffers)
    {
        bufferList[bufferIndex++] = buffer.get().getHandle();
    }

    return bindUniformBuffers(binding, index, bufferList, offsets, ranges);
}

DescriptorSet& DescriptorSet::bindUniformBuffers(
    const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    VKW_ASSERT(offsets.empty() || (offsets.size() == buffers.size()));
    VKW_ASSERT(ranges.empty() || (ranges.size() == buffers.size()));

    auto bufferInfo = utils::ScopedAllocator::allocateArray<VkDescriptorBufferInfo>(buffers.size());

    for(size_t i = 0; i < buffers.size(); ++i)
    {
        bufferInfo[i]
            = {buffers[i], offsets.empty() ? 0 : offsets[i], ranges.empty() ? VK_WHOLE_SIZE : ranges[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(buffers.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = bufferInfo.data();
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindStorageBuffer(
    const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset,
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

DescriptorSet& DescriptorSet::bindStorageBuffers(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<BaseBuffer>>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    auto bufferList = utils::ScopedAllocator::allocateArray<VkBuffer>(buffers.size());

    size_t bufferIndex = 0;
    for(const auto& buffer : buffers)
    {
        bufferList[bufferIndex++] = buffer.get().getHandle();
    }

    return bindStorageBuffers(binding, index, bufferList, offsets, ranges);
}

DescriptorSet& DescriptorSet::bindStorageBuffers(
    const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    VKW_ASSERT(offsets.empty() || (offsets.size() == buffers.size()));
    VKW_ASSERT(ranges.empty() || (ranges.size() == buffers.size()));

    auto bufferInfo = utils::ScopedAllocator::allocateArray<VkDescriptorBufferInfo>(buffers.size());

    for(size_t i = 0; i < buffers.size(); ++i)
    {
        bufferInfo[i]
            = {buffers[i], offsets.empty() ? 0 : offsets[i], ranges.empty() ? VK_WHOLE_SIZE : ranges[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(buffers.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = bufferInfo.data();
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindUniformBufferDynamic(
    const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset,
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
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindUniformBuffersDynamic(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<BaseBuffer>>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    auto bufferList = utils::ScopedAllocator::allocateArray<VkBuffer>(buffers.size());

    size_t bufferIndex = 0;
    for(const auto& buffer : buffers)
    {
        bufferList[bufferIndex++] = buffer.get().getHandle();
    }

    return bindUniformBuffersDynamic(binding, index, bufferList, offsets, ranges);
}

DescriptorSet& DescriptorSet::bindUniformBuffersDynamic(
    const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    VKW_ASSERT(offsets.empty() || (offsets.size() == buffers.size()));
    VKW_ASSERT(ranges.empty() || (ranges.size() == buffers.size()));

    auto bufferInfo = utils::ScopedAllocator::allocateArray<VkDescriptorBufferInfo>(buffers.size());

    for(size_t i = 0; i < buffers.size(); ++i)
    {
        bufferInfo[i]
            = {buffers[i], offsets.empty() ? 0 : offsets[i], ranges.empty() ? VK_WHOLE_SIZE : ranges[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(buffers.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = bufferInfo.data();
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindStorageBufferDynamic(
    const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset,
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
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindStorageBuffersDynamic(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<BaseBuffer>>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    auto bufferList = utils::ScopedAllocator::allocateArray<VkBuffer>(buffers.size());

    size_t bufferIndex = 0;
    for(const auto& buffer : buffers)
    {
        bufferList[bufferIndex++] = buffer.get().getHandle();
    }

    return bindStorageBuffersDynamic(binding, index, bufferList, offsets, ranges);
}

DescriptorSet& DescriptorSet::bindStorageBuffersDynamic(
    const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
    const std::span<VkDeviceSize>& offsets, const std::span<VkDeviceSize>& ranges)
{
    VKW_ASSERT(offsets.empty() || (offsets.size() == buffers.size()));
    VKW_ASSERT(ranges.empty() || (ranges.size() == buffers.size()));

    auto bufferInfo = utils::ScopedAllocator::allocateArray<VkDescriptorBufferInfo>(buffers.size());

    for(size_t i = 0; i < buffers.size(); ++i)
    {
        bufferInfo[i]
            = {buffers[i], offsets.empty() ? 0 : offsets[i], ranges.empty() ? VK_WHOLE_SIZE : ranges[i]};
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(buffers.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = bufferInfo.data();
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

DescriptorSet& DescriptorSet::bindAccelerationStructure(
    const uint32_t binding, const uint32_t index, const VkAccelerationStructureKHR accelerationStructure)
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
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}

DescriptorSet& DescriptorSet::bindAccelerationStructures(
    const uint32_t binding, const uint32_t index,
    const std::initializer_list<std::reference_wrapper<TopLevelAccelerationStructure>>&
        accelerationStructures)
{
    auto asList
        = utils::ScopedAllocator::allocateArray<VkAccelerationStructureKHR>(accelerationStructures.size());

    size_t asIndex = 0;
    for(const auto& as : accelerationStructures)
    {
        asList[asIndex++] = as.get().getHandle();
    }

    return bindAccelerationStructures(binding, index, asList);
}

DescriptorSet& DescriptorSet::bindAccelerationStructures(
    const uint32_t binding, const uint32_t index,
    const std::span<VkAccelerationStructureKHR>& accelerationStructures)
{
    VkWriteDescriptorSetAccelerationStructureKHR asWriteDescriptorSet = {};
    asWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    asWriteDescriptorSet.pNext = nullptr;
    asWriteDescriptorSet.accelerationStructureCount = static_cast<uint32_t>(accelerationStructures.size());
    asWriteDescriptorSet.pAccelerationStructures = accelerationStructures.data();

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = &asWriteDescriptorSet;
    writeDescriptorSet.dstSet = descriptorSet_;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = index;
    writeDescriptorSet.descriptorCount = static_cast<uint32_t>(accelerationStructures.size());
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkUpdateDescriptorSets(device_->getHandle(), 1, &writeDescriptorSet, 0, nullptr);
    return *this;
}
} // namespace vkw
