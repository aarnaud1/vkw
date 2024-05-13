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

#include <memory>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include <vulkan/vulkan.h>

#include "utils.hpp"
#include "Instance.hpp"
#include "Device.hpp"
#include "IMemoryObject.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

namespace vk
{
class Memory
{
public:
  Memory() = delete;

  Memory(Device &device, VkMemoryPropertyFlags properties, bool external = false);

  ~Memory();

  template <typename T>
  Buffer<T> &createBuffer(
      VkBufferUsageFlags usage, const size_t elements,
      VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
  {
    managedObjects_.emplace_back(
        ObjectPtr(new Buffer<T>(device_, elements, usage, properties_, sharingMode, external_)));
    auto &ptr = managedObjects_.back();
    return *static_cast<Buffer<T> *>(ptr.get());
  }

  template <ImageFormat imgFormat, typename T>
  Image<imgFormat, T> &createImage(
      VkImageType imageType, VkExtent3D extent, VkImageUsageFlags usage, uint32_t numLayers = 1,
      VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, uint32_t mipLevels = 1,
      VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
  {
    managedObjects_.emplace_back(ObjectPtr(new Image<imgFormat, T>(
        device_, imageType, extent, usage, properties_, numLayers, tiling, mipLevels,
        sharingMode)));
    auto &ptr = managedObjects_.back();
    return *static_cast<Image<imgFormat, T> *>(ptr.get());
  }

  void allocate();

  VkDeviceMemory &getHandle() { return memory_; }

  Device &getDevice() { return device_; }

  uint32_t getSize() const { return size_; }

  bool isMappable() { return properties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }

  template <typename T>
  void copyFromHost(void *hostPtr, size_t offset, size_t size)
  {
    if(properties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
      const size_t nBytes = size * sizeof(T);
      void *data = nullptr;
      vkMapMemory(this->device_.getHandle(), this->memory_, offset, nBytes, 0, &data);
      memcpy(data, hostPtr, nBytes);

      if(!(properties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
      {
        VkMappedMemoryRange range;
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext = nullptr;
        range.memory = memory_;
        range.offset = offset;
        range.size = nBytes;
        vkFlushMappedMemoryRanges(device_.getHandle(), 1, &range);
      }

      vkUnmapMemory(this->device_.getHandle(), this->memory_);
    }
    else
    {
      fprintf(stderr, "Error, memory is not visible by host\n");
      exit(1);
    }
  }

  template <typename T>
  void copyFromDevice(void *hostPtr, size_t offset, size_t size)
  {
    const size_t nBytes = size * sizeof(T);
    if(properties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
      void *data = nullptr;
      vkMapMemory(this->device_.getHandle(), this->memory_, offset, nBytes, 0, &data);
      memcpy(hostPtr, data, nBytes);

      if(!(properties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
      {
        VkMappedMemoryRange range;
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.pNext = nullptr;
        range.memory = memory_;
        range.offset = offset;
        range.size = nBytes;
        vkFlushMappedMemoryRanges(device_.getHandle(), 1, &range);
      }

      vkUnmapMemory(this->device_.getHandle(), this->memory_);
    }
    else
    {
      fprintf(stderr, "Error, memory is not visible by host\n");
      exit(1);
    }
  }

  int getExternalMemHandle();

private:
  using ObjectPtr = std::unique_ptr<IMemoryObject>;

  Device &device_;
  VkMemoryPropertyFlags properties_;
  bool external_;
  VkDeviceMemory memory_;
  std::vector<ObjectPtr> managedObjects_;
  uint32_t size_;

  uint32_t findMemoryType(VkMemoryPropertyFlags properties);

  uint32_t computeSize();
};
} // namespace vk
