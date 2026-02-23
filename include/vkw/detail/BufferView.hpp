/*
 * Copyright (c) 2026 Adrien ARNAUD
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
        const Device& device, const Buffer<T, memType>& buffer, const VkFormat format,
        const VkDeviceSize offset = 0, const VkDeviceSize range = VK_WHOLE_SIZE,
        const void* pCreateNext = nullptr)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(device, buffer, format, offset, range, pCreateNext), "Error creating buffer view");
    }
    BufferView(const Device& device, const VkBufferViewCreateInfo& createInfo)
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
        const Device& device, const Buffer<T, memType>& buffer, const VkFormat format,
        const VkDeviceSize offset = 0, const VkDeviceSize range = VK_WHOLE_SIZE,
        const void* pCreateNext = nullptr)
    {
        VKW_ASSERT(this->initialized() == false);

        device_ = &device;

        VkBufferViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        createInfo.pNext = pCreateNext;
        createInfo.flags = 0;
        createInfo.buffer = buffer.getHandle();
        createInfo.format = format;
        createInfo.offset = offset;
        createInfo.range = range;
        VKW_CHECK_VK_RETURN_FALSE(
            device_->vk().vkCreateBufferView(device_->getHandle(), &createInfo, nullptr, &bufferView_));

        initialized_ = true;

        return true;
    }

    bool init(const Device& device, const VkBufferViewCreateInfo& createInfo)
    {
        VKW_ASSERT(this->initialized() == false);

        device_ = &device;

        VKW_CHECK_VK_RETURN_FALSE(
            device_->vk().vkCreateBufferView(device_->getHandle(), &createInfo, nullptr, &bufferView_));

        initialized_ = true;

        return true;
    }

    void clear()
    {
        VKW_DELETE_VK(BufferView, bufferView_);
        device_ = nullptr;
        initialized_ = false;
    }

    bool initialized() const { return initialized_; }

    VkBufferView getHandle() const { return bufferView_; }

  private:
    const Device* device_{nullptr};
    VkBufferView bufferView_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
