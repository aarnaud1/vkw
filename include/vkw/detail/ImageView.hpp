/*
 * Copyright (c) 2025 Adrien ARNAUD
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

#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Image.hpp"
#include "vkw/detail/Instance.hpp"

#include <cstdio>
#include <cstdlib>

namespace vkw
{
class ImageView
{
  public:
    ImageView() {}

    template <MemoryType memType>
    ImageView(
        const Device& device,
        const Image<memType>& img,
        const VkImageViewType viewType,
        const VkFormat format,
        const VkImageSubresourceRange& subresourceRange,
        const void* pCreateNext = nullptr)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(device, img, viewType, format, subresourceRange, pCreateNext),
            "Initializing image view");
    }

    ImageView(Device& device, const VkImageViewCreateInfo& createInfo)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, createInfo), "Initializing image view");
    }

    ImageView(const ImageView&) = delete;
    ImageView(ImageView&& rhs) { *this = std::move(rhs); }

    ImageView& operator=(const ImageView&&) = delete;
    ImageView& operator=(ImageView&& rhs)
    {
        this->clear();
        std::swap(device_, rhs.device_);
        std::swap(imageView_, rhs.imageView_);
        std::swap(initialized_, rhs.initialized_);
        return *this;
    }

    ~ImageView() { clear(); }

    template <MemoryType memType>
    bool init(
        const Device& device,
        const Image<memType>& img,
        const VkImageViewType viewType,
        const VkFormat format,
        const VkImageSubresourceRange subresourceRange,
        const void* pCreateNext = nullptr)
    {
        if(!initialized_)
        {
            device_ = &device;

            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.pNext = pCreateNext;
            createInfo.flags = 0;
            createInfo.image = img.getHandle();
            createInfo.viewType = viewType;
            createInfo.format = format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            createInfo.subresourceRange = subresourceRange;
            VKW_INIT_CHECK_VK(device_->vk().vkCreateImageView(
                device_->getHandle(), &createInfo, nullptr, &imageView_));

            initialized_ = true;
        }

        return true;
    }

    bool init(const Device& device, const VkImageViewCreateInfo& createInfo)
    {
        if(!initialized_)
        {
            device_ = &device;

            VKW_INIT_CHECK_VK(device_->vk().vkCreateImageView(
                device_->getHandle(), &createInfo, nullptr, &imageView_));

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        VKW_DELETE_VK(ImageView, imageView_);
        device_ = nullptr;
        initialized_ = false;
    }

    bool initialized() const { return initialized_; }

    VkImageView getHandle() const { return imageView_; }

  private:
    const Device* device_{nullptr};
    VkImageView imageView_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
