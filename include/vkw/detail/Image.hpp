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
#include "vkw/detail/MemoryCommon.hpp"
#include "vkw/detail/utils.hpp"

namespace vkw
{
template <MemoryType memType, VkImageUsageFlags additionalFlags = 0>
class Image
{
  public:
    using MemFlagsType = MemoryFlags<memType>;

    Image() {}
    explicit Image(
        Device& device,
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkImageCreateFlags createFlags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        void* pCreateNext = nullptr)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(
                device,
                imageType,
                format,
                extent,
                usage,
                numLayers,
                tiling,
                mipLevels,
                createFlags,
                sharingMode,
                pCreateNext),
            "Error creating image");
    }

    explicit Image(Device& device, const VkImageCreateInfo& createInfo)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, createInfo), "Error creating image");
    }

    Image(const Image&) = delete;
    Image(Image&& rhs) { *this = std::move(rhs); };

    Image& operator=(const Image&&) = delete;
    Image& operator=(Image&& rhs)
    {
        this->clear();

        std::swap(image_, rhs.image_);

        std::swap(format_, rhs.format_);
        std::swap(extent_, rhs.extent_);
        std::swap(usage_, rhs.usage_);

        std::swap(allocInfo_, rhs.allocInfo_);
        std::swap(memAllocation_, rhs.memAllocation_);

        std::swap(device_, rhs.device_);
        std::swap(initialized_, rhs.initialized_);

        return *this;
    }

    ~Image() { this->clear(); }

    bool init(
        Device& device,
        VkImageType imageType,
        VkFormat format,
        VkExtent3D extent,
        VkImageUsageFlags usage,
        uint32_t numLayers = 1,
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
        uint32_t mipLevels = 1,
        VkImageCreateFlags createFlags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        void* pCreateNext = nullptr)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->format_ = format;
            this->extent_ = extent;
            this->usage_ = usage | additionalFlags;

            VkImageCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = createFlags;
            createInfo.imageType = imageType;
            createInfo.format = format;
            createInfo.extent = extent;
            createInfo.mipLevels = mipLevels;
            createInfo.arrayLayers = numLayers;
            createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            createInfo.tiling = tiling;
            createInfo.usage = usage;
            createInfo.sharingMode = sharingMode;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
            createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            createInfo.pNext = pCreateNext;

            VmaAllocationCreateInfo allocationCreateInfo = {};
            allocationCreateInfo.flags = MemFlagsType::allocationFlags;
            allocationCreateInfo.usage = MemFlagsType::usage;
            allocationCreateInfo.requiredFlags = MemFlagsType::requiredFlags;
            allocationCreateInfo.preferredFlags = MemFlagsType::preferredFlags;
            allocationCreateInfo.memoryTypeBits = 0;
            allocationCreateInfo.pool = VK_NULL_HANDLE;
            allocationCreateInfo.pUserData = nullptr;
            allocationCreateInfo.priority = 1.0f;
            VKW_INIT_CHECK_VK(vmaCreateImage(
                device_->allocator(),
                &createInfo,
                &allocationCreateInfo,
                &image_,
                &memAllocation_,
                &allocInfo_));

            utils::Log::Debug("vkw", "Image created");
            utils::Log::Debug("vkw", "  deviceLocal:  %s", deviceLocal() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostVisible:  %s", hostVisible() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostCoherent: %s", hostCoherent() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostCached:   %s", hostCached() ? "True" : "False");

            initialized_ = true;
        }
        return true;
    }

    bool init(Device& device, const VkImageCreateInfo& createInfo)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->format_ = createInfo.format;
            this->extent_ = createInfo.extent;
            this->usage_ = createInfo.usage | additionalFlags;

            VkImageCreateInfo imgCreateInfo = createInfo;
            imgCreateInfo.usage = usage_;

            VmaAllocationCreateInfo allocationCreateInfo = {};
            allocationCreateInfo.flags = MemFlagsType::allocationFlags;
            allocationCreateInfo.usage = MemFlagsType::usage;
            allocationCreateInfo.requiredFlags = MemFlagsType::requiredFlags;
            allocationCreateInfo.preferredFlags = MemFlagsType::preferredFlags;
            allocationCreateInfo.memoryTypeBits = 0;
            allocationCreateInfo.pool = VK_NULL_HANDLE;
            allocationCreateInfo.pUserData = nullptr;
            allocationCreateInfo.priority = 1.0f;
            VKW_INIT_CHECK_VK(vmaCreateImage(
                device_->allocator(),
                &imgCreateInfo,
                &allocationCreateInfo,
                &image_,
                &memAllocation_,
                &allocInfo_));

            utils::Log::Debug("vkw", "Image created");
            utils::Log::Debug("vkw", "  deviceLocal:  %s", deviceLocal() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostVisible:  %s", hostVisible() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostCoherent: %s", hostCoherent() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostCached:   %s", hostCached() ? "True" : "False");

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        if(image_ != VK_NULL_HANDLE)
        {
            vmaDestroyImage(device_->allocator(), image_, memAllocation_);
            image_ = VK_NULL_HANDLE;
            memAllocation_ = VK_NULL_HANDLE;
        }

        format_ = {};
        extent_ = {};
        usage_ = {};

        allocInfo_ = {};

        device_ = nullptr;
        initialized_ = false;
    }

    VkBufferUsageFlags getUsage() const { return usage_; }
    VkExtent3D getSize() const { return extent_; }
    VkFormat getFormat() const { return format_; }

    VkImage getHandle() const { return image_; }

    // Memory properties
    bool deviceLocal() const
    {
        return device_->getMemProperties().memoryTypes[allocInfo_.memoryType].propertyFlags
               & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    bool hostVisible() const
    {
        return device_->getMemProperties().memoryTypes[allocInfo_.memoryType].propertyFlags
               & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    bool hostCoherent() const
    {
        return device_->getMemProperties().memoryTypes[allocInfo_.memoryType].propertyFlags
               & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    bool hostCached() const
    {
        return device_->getMemProperties().memoryTypes[allocInfo_.memoryType].propertyFlags
               & VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

  private:
    Device* device_{nullptr};

    VkFormat format_{};
    VkExtent3D extent_{};
    VkImageUsageFlags usage_{};
    VkImage image_{VK_NULL_HANDLE};

    VmaAllocationInfo allocInfo_{};
    VmaAllocation memAllocation_{VK_NULL_HANDLE};

    bool initialized_{false};
};

template <VkImageUsageFlags additionalFlags = 0>
using DeviceImage = Image<MemoryType::Device, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostImage = Image<MemoryType::Host, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostStagingImage = Image<MemoryType::HostStaging, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostDeviceImage = Image<MemoryType::HostDevice, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostToDeviceImage = Image<MemoryType::TransferHostDevice, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using DeviceToHostImage = Image<MemoryType::TransferDeviceHost, additionalFlags>;
} // namespace vkw