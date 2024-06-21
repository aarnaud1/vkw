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
struct ImagePropertyFlags
{
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags memoryFlags;
};

class Image : public IMemoryObject
{
  public:
    Image(){};

    Image(
        Device &device,
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memProperties,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
    {
        this->init(
            device,
            imageType,
            format,
            extent,
            usage,
            memProperties,
            numLayers,
            tiling,
            mipLevels,
            sharingMode,
            external);
    }

    Image(
        Device &device,
        VkImageType imageType,
        VkFormat format,
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
            format,
            extent,
            flags.usage,
            flags.memoryFlags,
            numLayers,
            tiling,
            mipLevels,
            sharingMode,
            external)
    {}

    Image(const Image &) = delete;
    Image(Image &&cp) { *this = std::move(cp); }

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&cp)
    {
        this->clear();

        std::swap(device_, cp.device_);

        std::swap(format_, cp.format_);
        std::swap(extent_, cp.extent_);
        std::swap(memProperties_, cp.memProperties_);
        std::swap(memRequirements_, cp.memRequirements_);
        std::swap(usage_, cp.usage_);
        std::swap(image_, cp.image_);

        std::swap(offset_, cp.offset_);

        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    ~Image() { this->clear(); }

    void init(
        Device &device,
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memProperties,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        bool external = false)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->format_ = format;
            this->extent_ = extent;
            this->memProperties_ = memProperties;
            this->usage_ = usage;

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

            initialized_ = true;
        }
    }

    void clear()
    {
        if(image_ != VK_NULL_HANDLE)
        {
            vkDestroyImage(device_->getHandle(), image_, nullptr);
        }

        device_ = nullptr;

        extent_ = {};
        format_ = {};
        memProperties_ = {};
        memRequirements_ = {};
        usage_ = {};
        image_ = VK_NULL_HANDLE;

        offset_ = 0;

        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

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

    VkFormat getFormat() const { return format_; }

    VkImage &getHandle() { return image_; }
    const VkImage &getHandle() const { return image_; }

  private:
    Device *device_{nullptr};

    VkFormat format_{};
    VkExtent3D extent_{};
    VkMemoryPropertyFlags memProperties_{};
    VkMemoryRequirements memRequirements_{};
    VkBufferUsageFlags usage_{};
    VkImage image_{VK_NULL_HANDLE};

    size_t offset_{0};

    bool initialized_{false};
};
} // namespace vkw
