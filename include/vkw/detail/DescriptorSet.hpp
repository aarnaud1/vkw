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

#pragma once

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/BufferView.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/DescriptorPool.hpp"
#include "vkw/detail/DescriptorSetLayout.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/ImageView.hpp"
#include "vkw/detail/Sampler.hpp"
#include "vkw/detail/TopLevelAccelerationStructure.hpp"

#include <cstdlib>
#include <vector>

namespace vkw
{
class DescriptorSet
{
  public:
    DescriptorSet() {}
    DescriptorSet(
        const Device& device, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool,
        const void* pCreateNext = nullptr);

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&& rhs) { *this = std::move(rhs); };

    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet& operator=(DescriptorSet&& rhs);

    ~DescriptorSet();

    bool init(
        const Device& device, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool,
        const void* pCreateNext = nullptr);

    void clear();

    bool initialized() const { return initialized_; }

    DescriptorSet& bindSampler(const uint32_t binding, const Sampler& sampler)
    {
        return bindSampler(binding, sampler.getHandle());
    }

    DescriptorSet& bindCombinedImageSampler(
        const uint32_t binding, const Sampler& sampler, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindCombinedImageSampler(binding, sampler.getHandle(), imageView.getHandle(), layout);
    }

    DescriptorSet& bindSampledImage(
        const uint32_t binding, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindSampledImage(binding, imageView.getHandle(), layout);
    }

    DescriptorSet& bindStorageImage(
        const uint32_t binding, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindStorageImage(binding, imageView.getHandle(), layout);
    }
    DescriptorSet& bindStorageImageIndex(
        const uint32_t binding, const ImageView& imageView, const uint32_t index,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindStorageImageIndex(binding, imageView.getHandle(), index, layout);
    }

    DescriptorSet& bindUniformTexelBuffer(const uint32_t binding, const BufferView& bufferView)
    {
        return bindUniformTexelBuffer(binding, bufferView.getHandle());
    }

    DescriptorSet& bindStorageTexelBuffer(const uint32_t binding, const BufferView& bufferView)
    {
        return bindStorageTexelBuffer(binding, bufferView.getHandle());
    }

    template <typename BufferType>
    DescriptorSet& bindUniformBuffer(
        const uint32_t binding, const BufferType& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        using T = typename BufferType::value_type;
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindUniformBuffer(binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    template <typename BufferType>
    DescriptorSet& bindStorageBuffer(
        const uint32_t binding, const BufferType& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        using T = typename BufferType::value_type;
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindStorageBuffer(binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }
    template <typename BufferType>
    DescriptorSet& bindStorageBufferIndex(
        const uint32_t binding, const BufferType& buffer, const uint32_t index, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        using T = typename BufferType::value_type;
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindStorageBufferIndex(binding, buffer.getHandle(), index, offset * sizeof(T), bufferRange);
    }

    template <typename BufferType>
    DescriptorSet& bindUniformBufferDynamic(
        const uint32_t binding, const BufferType& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        using T = typename BufferType::value_type;
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindUniformBufferDynamic(binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    template <typename BufferType>
    DescriptorSet& bindStorageBufferDynamic(
        const uint32_t binding, const BufferType& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        using T = typename BufferType::value_type;
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindStorageBufferDynamic(binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    DescriptorSet& bindAccelerationStructure(
        const uint32_t binding, const TopLevelAccelerationStructure& tlas)
    {
        return bindAccelerationStructure(binding, tlas.getHandle());
    }

    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindSampler(const uint32_t binding, const VkSampler sampler);

    DescriptorSet& bindCombinedImageSampler(
        uint32_t binding, const VkSampler sampler, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindSampledImage(
        const uint32_t binding, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindStorageImage(
        const uint32_t binding, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);
    DescriptorSet& bindStorageImageIndex(
        const uint32_t binding, const VkImageView imageView, const uint32_t index,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindUniformTexelBuffer(const uint32_t binding, const VkBufferView& bufferView);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindStorageTexelBuffer(const uint32_t binding, const VkBufferView& bufferView);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindStorageBuffer(
        const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);
    DescriptorSet& bindStorageBufferIndex(
        const uint32_t binding, const VkBuffer buffer, const uint32_t index, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindUniformBuffer(
        const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindStorageBufferDynamic(
        const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindUniformBufferDynamic(
        const uint32_t binding, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);
    /// @todo: Add bindAccelerationStructureIndex()

    DescriptorSet& bindAccelerationStructure(
        const uint32_t binding, const VkAccelerationStructureKHR accelerationStructure);
    /// @todo: Add bindAccelerationStructureIndex()

    VkDescriptorSet getHandle() const { return descriptorSet_; }

  private:
    const Device* device_{nullptr};
    const DescriptorPool* descriptorPool_{nullptr};

    VkDescriptorSet descriptorSet_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw