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
#include "vkWrappers/wrappers/Image.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <vulkan/vulkan.h>

namespace vk
{
class ImageView
{
  public:
    ImageView() {}

    ImageView(
        Device &device,
        Image &img,
        VkImageViewType viewType,
        VkFormat format,
        VkImageSubresourceRange subresourceRange)
    {
        this->init(device, img, viewType, format, subresourceRange);
    }

    ImageView(const ImageView &) = delete;
    ImageView(ImageView &&cp) { *this = std::move(cp); }

    ImageView &operator=(const ImageView &&) = delete;
    ImageView &operator=(ImageView &&cp)
    {
        clear();
        std::swap(device_, cp.device_);
        std::swap(imageView_, cp.imageView_);
        std::swap(initialized_, cp.initialized_);
        return *this;
    }

    ~ImageView() { clear(); }

    void init(
        Device &device,
        Image &img,
        VkImageViewType viewType,
        VkFormat format,
        VkImageSubresourceRange subresourceRange)
    {
        if(!initialized_)
        {
            device_ = &device;

            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.image = img.getHandle();
            createInfo.viewType = viewType;
            createInfo.format = format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            createInfo.subresourceRange = subresourceRange;

            CHECK_VK(
                vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageView_),
                "Creating image view");

            initialized_ = true;
        }
    }

    void clear()
    {
        if(imageView_ != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device_->getHandle(), imageView_, nullptr);
        }

        device_ = nullptr;
        imageView_ = VK_NULL_HANDLE;

        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    VkImageView &getHandle() { return imageView_; }
    const VkImageView &getHandle() const { return imageView_; }

  private:
    Device *device_{nullptr};
    VkImageView imageView_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vk
