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
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace vkw
{
class Image final : public IMemoryObject
{
  public:
    Image(){};

    Image(const Image &) = default;
    Image(Image &&) = default;

    Image &operator=(const Image &) = default;
    Image &operator=(Image &&) = default;

    ~Image() {}

    VkBufferUsageFlags getUsage() const { return usage_; }
    VkExtent3D getSize() const { return extent_; }
    VkFormat getFormat() const { return format_; }

    VkImage &getHandle() { return image_; }
    const VkImage &getHandle() const { return image_; }

  private:
    friend class Memory;

    Device *device_{nullptr};

    VkFormat format_{};
    VkExtent3D extent_{};
    VkImageUsageFlags usage_{};

    VkImage image_{VK_NULL_HANDLE};

    void clear() override
    {
        memAlign_ = 0;
        memSize_ = 0;
        memOffset_ = 0;

        memTypeBits_ = 0;

        if(image_ != VK_NULL_HANDLE)
        {
            vkDestroyImage(device_->getHandle(), image_, nullptr);
        }

        device_ = nullptr;

        extent_ = {};
        format_ = {};
        usage_ = {};
        image_ = VK_NULL_HANDLE;
    }

    bool bindResource(VkDeviceMemory mem, const size_t offset) override
    {
        VkResult res = vkBindImageMemory(device_->getHandle(), image_, mem, offset);
        if(res != VK_SUCCESS)
        {
            utils::Log::Error("vkw::Image", "Error binding memory - %s", string_VkResult(res));
            return false;
        }
        return true;
    }
};
} // namespace vkw
