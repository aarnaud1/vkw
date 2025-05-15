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

#include "vkw/wrappers/Common.hpp"
#include "vkw/wrappers/Device.hpp"
#include "vkw/wrappers/Image.hpp"
#include "vkw/wrappers/Instance.hpp"

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
        Device& device,
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
        Device& device,
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

    bool init(Device& device, const VkImageViewCreateInfo& createInfo)
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

    bool isInitialized() const { return initialized_; }

    VkImageView getHandle() const { return imageView_; }

  private:
    Device* device_{nullptr};
    VkImageView imageView_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
