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

#pragma once

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/BufferView.hpp"
#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/ImageView.hpp"
#include "vkWrappers/wrappers/Sampler.hpp"
#include "vkWrappers/wrappers/TopLevelAccelerationStructure.hpp"

#include <cstdlib>
#include <vector>

namespace vkw
{
class DescriptorSet
{
  public:
    DescriptorSet() {}

    DescriptorSet(const DescriptorSet&) = default;
    DescriptorSet(DescriptorSet&&) = default;

    DescriptorSet& operator=(const DescriptorSet&) = default;
    DescriptorSet& operator=(DescriptorSet&&) = default;

    ~DescriptorSet() = default;

    DescriptorSet& bindSampler(const uint32_t binding, const Sampler& sampler)
    {
        return bindSampler(binding, sampler.getHandle());
    }

    DescriptorSet& bindCombinedImageSampler(
        const uint32_t binding,
        const Sampler& sampler,
        const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindCombinedImageSampler(
            binding, sampler.getHandle(), imageView.getHandle(), layout);
    }

    DescriptorSet& bindSampledImage(
        const uint32_t binding,
        const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindSampledImage(binding, imageView.getHandle(), layout);
    }

    DescriptorSet& bindStorageImage(
        const uint32_t binding,
        const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindStorageImage(binding, imageView.getHandle(), layout);
    }

    DescriptorSet& bindUniformTexelBuffer(const uint32_t binding, const BufferView& bufferView)
    {
        return bindUniformTexelBuffer(binding, bufferView.getHandle());
    }

    DescriptorSet& bindStorageTexelBuffer(const uint32_t binding, const BufferView& bufferView)
    {
        return bindStorageTexelBuffer(binding, bufferView.getHandle());
    }

    template <typename T, MemoryType memType>
    DescriptorSet& bindUniformBuffer(
        const uint32_t binding,
        const Buffer<T, memType>& buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindUniformBuffer(binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    template <typename T, MemoryType memType>
    DescriptorSet& bindStorageBuffer(
        const uint32_t binding,
        const Buffer<T, memType>& buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindStorageBuffer(binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    template <typename T, MemoryType memType>
    DescriptorSet& bindUniformBufferDynamic(
        const uint32_t binding,
        const Buffer<T, memType>& buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindUniformBufferDynamic(
            binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    template <typename T, MemoryType memType>
    DescriptorSet& bindStorageBufferDynamic(
        const uint32_t binding,
        const Buffer<T, memType>& buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * sizeof(T);
        return bindStorageBufferDynamic(
            binding, buffer.getHandle(), offset * sizeof(T), bufferRange);
    }

    DescriptorSet& bindAccelerationStructure(
        const uint32_t binding, const TopLevelAccelerationStructure& tlas)
    {
        return bindAccelerationStructure(binding, tlas.getHandle());
    }

    // ---------------------------------------------------------------------------------------------

    DescriptorSet& bindSampler(const uint32_t binding, const VkSampler sampler);

    DescriptorSet& bindCombinedImageSampler(
        uint32_t binding,
        const VkSampler sampler,
        const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindSampledImage(
        const uint32_t binding,
        const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindStorageImage(
        uint32_t binding,
        const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindUniformTexelBuffer(const uint32_t binding, const VkBufferView& bufferView);

    DescriptorSet& bindStorageTexelBuffer(const uint32_t binding, const VkBufferView& bufferView);

    DescriptorSet& bindStorageBuffer(
        const uint32_t binding,
        const VkBuffer buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindUniformBuffer(
        const uint32_t binding,
        const VkBuffer buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindStorageBufferDynamic(
        const uint32_t binding,
        const VkBuffer buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindUniformBufferDynamic(
        const uint32_t binding,
        const VkBuffer buffer,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindAccelerationStructure(
        const uint32_t binding, const VkAccelerationStructureKHR accelerationStructure);

    VkDescriptorSet getHandle() const { return descriptorSet_; }

  private:
    friend class DescriptorPool;

    Device* device_{nullptr};
    VkDescriptorSet descriptorSet_{VK_NULL_HANDLE};
};
} // namespace vkw