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

#include <cstdio>
#include <cstdlib>
#include <vulkan/vulkan.h>

namespace vkw
{
class ImageView
{
  public:
    ImageView() {}
    ImageView(
        Device& device,
        Image& img,
        VkImageViewType viewType,
        VkFormat format,
        VkImageSubresourceRange subresourceRange);

    ImageView(const ImageView&) = delete;
    ImageView(ImageView&& cp) { *this = std::move(cp); }

    ImageView& operator=(const ImageView&&) = delete;
    ImageView& operator=(ImageView&& cp);

    ~ImageView() { clear(); }

    bool init(
        Device& device,
        Image& img,
        VkImageViewType viewType,
        VkFormat format,
        VkImageSubresourceRange subresourceRange);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkImageView& getHandle() { return imageView_; }
    const VkImageView& getHandle() const { return imageView_; }

  private:
    Device* device_{nullptr};
    VkImageView imageView_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
