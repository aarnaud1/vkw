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

#pragma once

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/ImageView.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class DescriptorSet
{
  public:
    DescriptorSet() {}

    DescriptorSet(const DescriptorSet &) = default;
    DescriptorSet(DescriptorSet &&) = default;

    DescriptorSet &operator=(const DescriptorSet &) = default;
    DescriptorSet &operator=(DescriptorSet &&) = default;

    ~DescriptorSet() = default;

    template <typename T>
    inline DescriptorSet &bindStorageBuffer(const uint32_t bindingPoint, const Buffer<T> &buffer)
    {
        return bindStorageBuffer(bindingPoint, buffer.getFullSizeInfo());
    }

    template <typename T>
    inline DescriptorSet &bindUniformBuffer(const uint32_t bindingPoint, const Buffer<T> &buffer)
    {
        return bindUniformBuffer(bindingPoint, buffer.getFullSizeInfo());
    }

    inline DescriptorSet &bindStorageImage(
        const uint32_t bindingPoint,
        const ImageView &image,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindStorageImage(
            bindingPoint, VkDescriptorImageInfo{nullptr, image.getHandle(), layout});
    }

    DescriptorSet &bindStorageBuffer(uint32_t bindingId, VkDescriptorBufferInfo bufferInfo);
    DescriptorSet &bindStorageImage(uint32_t bindingId, VkDescriptorImageInfo imageInfo);
    DescriptorSet &bindUniformBuffer(uint32_t bindingId, VkDescriptorBufferInfo bufferInfo);
    DescriptorSet &bindSamplerImage(uint32_t bindingId, VkDescriptorImageInfo imageInfo);

    VkDescriptorSet getHandle() const { return descriptorSet_; }

  private:
    friend class DescriptorPool;

    Device *device_{nullptr};
    VkDescriptorSet descriptorSet_{VK_NULL_HANDLE};
};
} // namespace vkw