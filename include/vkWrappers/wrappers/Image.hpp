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
// TODO : remove after having implemented external support
#include <stdexcept>

#include <vulkan/vulkan.h>

#include "vkWrappers/wrappers/utils.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Formats.hpp"

namespace vk
{
struct ImagePropertyFlags
{
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags memoryFlags;
};

template <ImageFormat imgFormat, typename T>
class Image : public IMemoryObject
{
  public:
    static constexpr VkFormat format = FormatType<imgFormat, T>::format;
    using DataType = T;

    Image() = delete;

    Image(
        Device &device,
        VkImageType imageType,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memProperties,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
        : device_(&device), extent_(extent), memProperties_(memProperties), usage_(usage)
    {
        if(external)
        {
            throw std::runtime_error("External image object not supported yet");
        }

        VkImageCreateInfo imgCreateInfo = {};
        imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imgCreateInfo.pNext = nullptr;
        imgCreateInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
        imgCreateInfo.imageType = imageType;
        imgCreateInfo.format = format;
        imgCreateInfo.extent = extent;
        imgCreateInfo.mipLevels = mipLevels;
        imgCreateInfo.arrayLayers = numLayers;
        imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imgCreateInfo.tiling = tiling;
        imgCreateInfo.usage = usage;
        imgCreateInfo.sharingMode = sharingMode;
        imgCreateInfo.queueFamilyIndexCount = 0;
        imgCreateInfo.pQueueFamilyIndices = nullptr;
        imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        CHECK_VK(
            vkCreateImage(device_->getHandle(), &imgCreateInfo, nullptr, &image_),
            "Creating image");

        vkGetImageMemoryRequirements(device_->getHandle(), image_, &memRequirements_);
    }

    Image(
        Device &device,
        VkImageType imageType,
        VkExtent3D extent,
        ImagePropertyFlags &flags,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
        : Image(
            device,
            imageType,
            extent,
            flags.usage,
            flags.memoryFlags,
            numLayers,
            tiling,
            mipLevels,
            sharingMode,
            external)
    {}

    Image(const Image &cp) = delete;

    Image(Image &&cp) = delete;

    Image &operator=(const Image &cp) = delete;

    Image &operator=(Image &&cp) = delete;

    ~Image() { vkDestroyImage(device_->getHandle(), image_, nullptr); }

    VkMemoryPropertyFlags getMemProperties() const { return memProperties_; }

    VkBufferUsageFlags getUsage() const { return usage_; }

    VkExtent3D getSize() const { return extent_; }

    VkMemoryRequirements getMemRequirements() const override { return memRequirements_; }

    void bindResource(VkDeviceMemory mem, const size_t offset) override
    {
        offset_ = offset;
        vkBindImageMemory(device_->getHandle(), image_, mem, offset);
    }

    size_t getOffset() const override { return offset_; }

    VkFormat getFormat() const { return format; }

    VkImage &getHandle() { return image_; }
    const VkImage &getHandle() const { return image_; }

  private:
    Device *device_{nullptr};
    VkExtent3D extent_;
    VkMemoryPropertyFlags memProperties_;
    VkBufferUsageFlags usage_;
    VkImage image_{VK_NULL_HANDLE};
    VkMemoryRequirements memRequirements_;
    size_t offset_;
};
} // namespace vk
