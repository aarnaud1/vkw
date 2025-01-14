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

#include "vkWrappers/wrappers/ImageView.hpp"

#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
ImageView::ImageView(
    Device& device,
    VkImage& img,
    VkImageViewType viewType,
    VkFormat format,
    VkImageSubresourceRange subresourceRange)
{
    VKW_CHECK_BOOL_THROW(
        this->init(device, img, viewType, format, subresourceRange), "Initializing image view");
}

ImageView& ImageView::operator=(ImageView&& cp)
{
    clear();
    std::swap(device_, cp.device_);
    std::swap(imageView_, cp.imageView_);
    std::swap(initialized_, cp.initialized_);
    return *this;
}

bool ImageView::init(
    Device& device,
    VkImage& img,
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
        createInfo.image = img;
        createInfo.viewType = viewType;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        createInfo.subresourceRange = subresourceRange;
        VKW_INIT_CHECK_VK(
            vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageView_));

        initialized_ = true;
    }

    return true;
}

void ImageView::clear()
{
    VKW_DELETE_VK(ImageView, imageView_);

    device_ = nullptr;
    initialized_ = false;
}
} // namespace vkw