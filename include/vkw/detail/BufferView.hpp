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

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"

#include <cstdio>
#include <cstdlib>

namespace vkw
{
class BufferView
{
  public:
    BufferView() {}

    template <typename T, MemoryType memType>
    BufferView(
        Device& device,
        const Buffer<T, memType>& buffer,
        const VkFormat format,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE,
        const void* pCreateNext = nullptr)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(device, buffer, format, offset, range, pCreateNext),
            "Error creating buffer view");
    }
    BufferView(Device& device, const VkBufferViewCreateInfo& createInfo)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, createInfo), "Error creating buffer view");
    }

    BufferView(const BufferView&) = delete;
    BufferView(BufferView&& rhs) { *this = std::move(rhs); }

    BufferView& operator=(const BufferView&) = delete;
    BufferView& operator=(BufferView&& rhs)
    {
        this->clear();
        std::swap(device_, rhs.device_);
        std::swap(bufferView_, rhs.bufferView_);
        std::swap(initialized_, rhs.initialized_);
        return *this;
    }

    ~BufferView() { clear(); }

    template <typename T, MemoryType memType>
    bool init(
        Device& device,
        const Buffer<T, memType>& buffer,
        const VkFormat format,
        const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE,
        const void* pCreateNext = nullptr)
    {
        if(!initialized_)
        {
            device_ = &device;

            VkBufferViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
            createInfo.pNext = pCreateNext;
            createInfo.flags = 0;
            createInfo.buffer = buffer.getHandle();
            createInfo.format = format;
            createInfo.offset = offset;
            createInfo.range = range;
            VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateBufferView(
                device_->getHandle(), &createInfo, nullptr, &bufferView_));

            initialized_ = true;
        }
        return true;
    }

    bool init(Device& device, const VkBufferViewCreateInfo& createInfo)
    {
        if(!initialized_)
        {
            device_ = &device;

            VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateBufferView(
                device_->getHandle(), &createInfo, nullptr, &bufferView_));

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        device_ = nullptr;
        VKW_DELETE_VK(BufferView, bufferView_);
        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    VkBufferView getHandle() const { return bufferView_; }

  private:
    Device* device_{nullptr};
    VkBufferView bufferView_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw