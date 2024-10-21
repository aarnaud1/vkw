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
#include "vkWrappers/wrappers/IMemoryObject.hpp"
#include "vkWrappers/wrappers/Image.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class Memory
{
  public:
    Memory() {}
    Memory(Device &device, const VkMemoryPropertyFlags properties);

    Memory(const Memory &) = delete;
    Memory(Memory &&cp);

    Memory &operator=(const Memory &) = delete;
    Memory &operator=(Memory &&cp);

    ~Memory();

    bool init(Device &device, VkMemoryPropertyFlags properties);

    void clear();

    template <typename T>
    Buffer<T> &createBuffer(
        VkBufferUsageFlags usage,
        const size_t elements,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        memObjects_.emplace_back(
            std::unique_ptr<IMemoryObject>(new Buffer<T>(*device_, elements, usage, sharingMode)));
        auto &ptr = memObjects_.back();
        return *static_cast<Buffer<T> *>(ptr.get());
    }

    Image &createImage(
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkImageCreateFlags createFlags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        memObjects_.emplace_back(std::unique_ptr<IMemoryObject>(new Image(
            *device_,
            imageType,
            format,
            extent,
            usage,
            numLayers,
            tiling,
            mipLevels,
            createFlags,
            sharingMode)));
        auto &ptr = memObjects_.back();
        return *static_cast<Image *>(ptr.get());
    }

    bool isInitialized() const { return initialized_; }

    bool allocate();

    void release();

    VkDeviceMemory &getHandle() { return memory_; }
    const VkDeviceMemory &getHandle() const { return memory_; }

    size_t allocatedSize() const { return allocatedSize_; }

    VkMemoryPropertyFlags getPropertyFlags() const { return propertyFlags_; }

    bool isHostVisible() const { return propertyFlags_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }

    template <typename T>
    bool copyFromHost(const void *hostPtr, size_t offset, size_t size)
    {
        const size_t nBytes = size * sizeof(T);
        void *data = nullptr;

        if(!(isHostVisible()))
        {
            return false;
        }

        // TODO : round mapping size for non coherent access
        vkMapMemory(this->device_->getHandle(), this->memory_, offset, nBytes, 0, &data);
        memcpy(data, hostPtr, nBytes);
        if(!(propertyFlags_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            VkMappedMemoryRange range;
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.pNext = nullptr;
            range.memory = memory_;
            range.offset = 0;
            range.size = VK_WHOLE_SIZE;
            vkFlushMappedMemoryRanges(device_->getHandle(), 1, &range);
        }

        vkUnmapMemory(this->device_->getHandle(), this->memory_);

        return true;
    }

    template <typename T>
    bool copyFromDevice(void *hostPtr, size_t offset, size_t size)
    {
        const size_t nBytes = size * sizeof(T);
        void *data = nullptr;

        if(!(isHostVisible()))
        {
            return false;
        }

        vkMapMemory(this->device_->getHandle(), this->memory_, offset, nBytes, 0, &data);
        memcpy(hostPtr, data, nBytes);
        if(!(propertyFlags_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        {
            VkMappedMemoryRange range;
            range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range.pNext = nullptr;
            range.memory = memory_;
            range.offset = offset;
            range.size = nBytes;
            vkFlushMappedMemoryRanges(device_->getHandle(), 1, &range);
        }

        vkUnmapMemory(this->device_->getHandle(), this->memory_);
        return true;
    }

  private:
    Device *device_{nullptr};

    VkDeviceSize allocatedSize_{0};
    VkMemoryPropertyFlags propertyFlags_{};
    VkDeviceMemory memory_{VK_NULL_HANDLE};

    std::vector<std::unique_ptr<IMemoryObject>> memObjects_{};

    bool initialized_{false};

    uint32_t findMemoryType(VkMemoryPropertyFlags properties);
};
} // namespace vkw