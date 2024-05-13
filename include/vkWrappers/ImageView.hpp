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

#include <cstdlib>
#include <cstdio>

#include <vulkan/vulkan.h>

#include "vkWrappers/utils.hpp"
#include "vkWrappers/Instance.hpp"
#include "vkWrappers/Device.hpp"
#include "vkWrappers/Formats.hpp"
#include "vkWrappers/Image.hpp"

namespace vk
{
template <class ImgType>
class ImageView
{
public:
  ImageView(
      Device &device, ImgType &img, VkImageViewType viewType, VkFormat format,
      VkImageSubresourceRange subresourceRange)
      : device_(device)
  {
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
        vkCreateImageView(device_.getHandle(), &createInfo, nullptr, &imageView_),
        "Creating image view");
  }

  ~ImageView() { vkDestroyImageView(device_.getHandle(), imageView_, nullptr); }

  VkImageView &getHandle() { return imageView_; }

private:
  Device &device_;
  VkImageView imageView_;
};
} // namespace vk
