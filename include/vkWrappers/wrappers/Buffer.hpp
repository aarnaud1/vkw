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

#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/IMemoryObject.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <vulkan/vulkan.h>

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
    Buffer() {}

    Buffer(
        Device &device,
        size_t size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memProperties,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
    {
        this->init(device, size, usage, memProperties, sharingMode, external);
    }

    Buffer(
        Device &device,
        size_t size,
        BufferPropertyFlags flags,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
        : Buffer(device, size, flags.usage, flags.memoryFlags, sharingMode, external)
    {}

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&cp) { *this = std::move(cp); }

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&cp)
    {
        this->clear();

        std::swap(device_, cp.device_);

        std::swap(memProperties_, cp.memProperties_);
        std::swap(memRequirements_, cp.memRequirements_);
        std::swap(usage_, cp.usage_);
        std::swap(buffer_, cp.buffer_);

        std::swap(size_, cp.size_);
        std::swap(offset_, cp.offset_);

        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    ~Buffer() { this->clear(); }

    void init(
        Device &device,
        size_t size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags memProperties,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->memProperties_ = memProperties;
            this->usage_ = usage;
            this->size_ = size;

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
                    vkCreateBuffer(device_->getHandle(), &createInfo, nullptr, &buffer_),
                    "Creating buffer object");
            }
            else
            {
                createInfo.pNext = nullptr;
                CHECK_VK(
                    vkCreateBuffer(device_->getHandle(), &createInfo, nullptr, &buffer_),
                    "Creating buffer object");
            }

            vkGetBufferMemoryRequirements(device_->getHandle(), buffer_, &memRequirements_);

            initialized_ = true;
        }
    }

    void clear()
    {
        if(buffer_ != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device_->getHandle(), buffer_, nullptr);
        }

        device_ = nullptr;

        memProperties_ = {};
        memRequirements_ = {};
        usage_ = {};
        buffer_ = VK_NULL_HANDLE;

        size_ = 0;
        offset_ = 0;

        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    VkMemoryPropertyFlags getMemProperties() const { return memProperties_; }

    VkBufferUsageFlags getUsage() const { return usage_; }

    size_t getSize() const { return size_; }

    size_t getSizeBytes() const { return size_ * sizeof(T); }

    VkMemoryRequirements getMemRequirements() const override { return memRequirements_; }

    VkDescriptorBufferInfo getFullSizeInfo() const { return {buffer_, 0, VK_WHOLE_SIZE}; }

    void bindResource(VkDeviceMemory mem, const size_t offset) override
    {
        offset_ = offset;
        vkBindBufferMemory(device_->getHandle(), buffer_, mem, offset);
    }

    size_t getOffset() const override { return offset_; }

    VkBuffer &getHandle() { return buffer_; }
    const VkBuffer &getHandle() const { return buffer_; }

  protected:
    Device *device_{nullptr};

    VkMemoryPropertyFlags memProperties_{};
    VkMemoryRequirements memRequirements_{};
    VkBufferUsageFlags usage_{};
    VkBuffer buffer_{VK_NULL_HANDLE};

    size_t size_{0};
    size_t offset_{0};

    bool initialized_{false};
};
} // namespace vk
