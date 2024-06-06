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
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk
{
class Memory
{
  public:
    Memory() {}
    Memory(Device &device, VkMemoryPropertyFlags properties, bool external = false);

    Memory(const Memory &) = delete;
    Memory(Memory &&cp);

    Memory &operator=(const Memory &) = delete;
    Memory &operator=(Memory &&cp);

    ~Memory();

    void init(Device &device, VkMemoryPropertyFlags properties, bool external = false);

    void clear();

    bool isInitialized() const { return initialized_; }

    template <typename T>
    Buffer<T> &createBuffer(
        VkBufferUsageFlags usage,
        const size_t elements,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        managedObjects_.emplace_back(ObjectPtr(
            new Buffer<T>(*device_, elements, usage, properties_, sharingMode, external_)));
        auto &ptr = managedObjects_.back();
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
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        managedObjects_.emplace_back(ObjectPtr(new Image(
            *device_,
            imageType,
            format,
            extent,
            usage,
            properties_,
            numLayers,
            tiling,
            mipLevels,
            sharingMode)));
        auto &ptr = managedObjects_.back();
        return *static_cast<Image *>(ptr.get());
    }

    void allocate();

    void release();

    VkDeviceMemory &getHandle() { return memory_; }
    const VkDeviceMemory &getHandle() const { return memory_; }

    uint32_t getSize() const { return size_; }

    bool isMappable() { return properties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }

    template <typename T>
    void copyFromHost(const void *hostPtr, size_t offset, size_t size)
    {
        if(properties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            const size_t nBytes = size * sizeof(T);
            void *data = nullptr;
            vkMapMemory(this->device_->getHandle(), this->memory_, offset, nBytes, 0, &data);
            memcpy(data, hostPtr, nBytes);

            if(!(properties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
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
            vkMapMemory(this->device_->getHandle(), this->memory_, offset, nBytes, 0, &data);
            memcpy(hostPtr, data, nBytes);

            if(!(properties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
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

    Device *device_{nullptr};
    VkMemoryPropertyFlags properties_{};
    bool external_{false};
    VkDeviceMemory memory_{VK_NULL_HANDLE};
    std::vector<ObjectPtr> managedObjects_{};
    uint32_t size_{0};

    bool initialized_{false};

    uint32_t findMemoryType(VkMemoryPropertyFlags properties);

    uint32_t computeSize();
};
} // namespace vk
