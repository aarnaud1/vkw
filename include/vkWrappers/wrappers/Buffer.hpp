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

namespace vkw
{
template <typename T>
class Buffer final : public IMemoryObject
{
  public:
    Buffer() {}

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    ~Buffer() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    VkBufferUsageFlags getUsage() const { return usage_; }

    size_t getSize() const { return size_; }
    size_t getSizeBytes() const { return size_ * sizeof(T); }

    VkDescriptorBufferInfo getFullSizeInfo() const { return {buffer_, 0, VK_WHOLE_SIZE}; }

    VkBuffer &getHandle() { return buffer_; }
    const VkBuffer &getHandle() const { return buffer_; }

  private:
    friend class Memory;

    Device *device_{nullptr};

    size_t size_{0};

    VkBufferUsageFlags usage_{};
    VkBuffer buffer_{VK_NULL_HANDLE};

    bool initialized_{false};

    Buffer(
        Device &device,
        size_t size,
        VkBufferUsageFlags usage,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        CHECK_BOOL_THROW(this->init(device, size, usage, sharingMode), "Initializing buffer");
    }

    bool init(
        Device &device,
        size_t size,
        VkBufferUsageFlags usage,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->usage_ = usage;
            this->size_ = size;

            VkBufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.size = size_ * sizeof(T);
            createInfo.usage = usage_;
            createInfo.sharingMode = sharingMode;
            VKW_INIT_CHECK_VK(vkCreateBuffer(device_->getHandle(), &createInfo, nullptr, &buffer_));

            VkMemoryRequirements memRequirements{};
            vkGetBufferMemoryRequirements(device_->getHandle(), buffer_, &memRequirements);

            this->memAlign_ = memRequirements.alignment;
            this->memSize_ = memRequirements.size;
            this->memTypeBits_ = memRequirements.memoryTypeBits;

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        memAlign_ = 0;
        memSize_ = 0;
        memOffset_ = 0;

        memTypeBits_ = 0;

        if(buffer_ != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device_->getHandle(), buffer_, nullptr);
        }

        device_ = nullptr;

        usage_ = {};
        buffer_ = VK_NULL_HANDLE;

        size_ = 0;

        initialized_ = false;
    }

    bool bindResource(VkDeviceMemory mem, const size_t offset) override
    {
        VkResult res = vkBindBufferMemory(device_->getHandle(), buffer_, mem, offset);
        if(res != VK_SUCCESS)
        {
            utils::Log::Error("vkw::Buffer", "Error binding memory - %s", string_VkResult(res));
            return false;
        }
        return true;
    }
};
} // namespace vkw
