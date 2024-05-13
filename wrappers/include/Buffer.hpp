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

#include <cstdlib>
#include <cstdio>

#include <vulkan/vulkan.h>

#include "utils.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "IMemoryObject.hpp"

namespace vk
{
struct BufferPropertyFlags
{
  VkBufferUsageFlags usage;
  VkMemoryPropertyFlags memoryFlags;
};

template <typename T>
class Buffer : public IMemoryObject
{
public:
  Buffer() = delete;

  Buffer(
      Device &device, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties,
      VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, bool external = false)
      : device_(device), size_(size), memProperties_(memProperties), usage_(usage)
  {
    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size_ * sizeof(T);
    createInfo.usage = usage_;
    createInfo.sharingMode = sharingMode;

    if(external)
    {
      VkExternalMemoryBufferCreateInfo externalInfo = {};
      externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO;
      externalInfo.pNext = nullptr;
      externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
      createInfo.pNext = &externalInfo;
      CHECK_VK(
          vkCreateBuffer(device_.getHandle(), &createInfo, nullptr, &buffer_),
          "Creating buffer object");
    }
    else
    {
      createInfo.pNext = nullptr;
      CHECK_VK(
          vkCreateBuffer(device_.getHandle(), &createInfo, nullptr, &buffer_),
          "Creating buffer object");
    }

    vkGetBufferMemoryRequirements(device_.getHandle(), buffer_, &memRequirements_);
  }

  Buffer(
      Device &device, size_t size, BufferPropertyFlags flags,
      VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE, bool external = false)
      : Buffer(device, size, flags.usage, flags.memoryFlags, sharingMode, external)
  {}

  Buffer(const Buffer &cp) = delete;

  Buffer(Buffer &&cp) = delete;

  Buffer &operator=(const Buffer &cp) = delete;

  Buffer &operator=(Buffer &&cp) = delete;

  ~Buffer() { vkDestroyBuffer(device_.getHandle(), buffer_, nullptr); }

  VkMemoryPropertyFlags getMemProperties() const { return memProperties_; }

  VkBufferUsageFlags getUsage() const { return usage_; }

  size_t getSize() const { return size_; }

  size_t getSizeBytes() const { return size_ * sizeof(T); }

  VkMemoryRequirements getMemRequirements() const override { return memRequirements_; }

  void bindResource(VkDeviceMemory mem, const size_t offset) override
  {
    offset_ = offset;
    vkBindBufferMemory(device_.getHandle(), buffer_, mem, offset);
  }

  size_t getOffset() const override { return offset_; }

  VkBuffer &getHandle() { return buffer_; }

  Device &getDevice() { return device_; }

protected:
  Device &device_;
  size_t size_;
  VkMemoryPropertyFlags memProperties_;
  VkBufferUsageFlags usage_;
  VkBuffer buffer_;
  VkMemoryRequirements memRequirements_;
  size_t offset_;
};
} // namespace vk
