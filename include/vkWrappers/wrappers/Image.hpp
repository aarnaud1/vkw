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
#include "vkWrappers/wrappers/MemoryCommon.hpp"
#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
template <MemoryType memType>
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
        VKW_CHECK_BOOL_THROW(
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
        VKW_CHECK_BOOL_THROW(this->init(device, createInfo), "Error creating image");
    }

    Image(const Image&) = delete;
    Image(Image&& rhs) { *this = std::move(rhs); };

    Image& operator=(const Image&&) = delete;
    Image& operator=(Image&& rhs)
    {
        this->clear();

        std::swap(image_, rhs.image_);
        std::swap(memory_, rhs.memory_);

        std::swap(format_, rhs.format_);
        std::swap(extent_, rhs.extent_);
        std::swap(usage_, rhs.usage_);

        std::swap(memRequirements_, rhs.memRequirements_);
        std::swap(memProperties_, rhs.memProperties_);

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
            this->usage_ = usage;

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
            VKW_INIT_CHECK_VK(vkCreateImage(device_->getHandle(), &createInfo, nullptr, &image_));
            VKW_INIT_CHECK_BOOL(allocateImageMemory());

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
            this->usage_ = createInfo.usage;

            VKW_INIT_CHECK_VK(vkCreateImage(device_->getHandle(), &createInfo, nullptr, &image_));
            VKW_INIT_CHECK_BOOL(allocateImageMemory());

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        VKW_DELETE_VK(Image, image_);
        VKW_FREE_VK(Memory, memory_);

        format_ = {};
        extent_ = {};
        usage_ = {};

        memRequirements_ = {};
        memProperties_ = {};

        device_ = nullptr;
        initialized_ = false;
    }

    VkBufferUsageFlags getUsage() const { return usage_; }
    VkExtent3D getSize() const { return extent_; }
    VkFormat getFormat() const { return format_; }

    VkImage& getHandle() { return image_; }
    const VkImage& getHandle() const { return image_; }

    // TODO: handle host images or disable them

    // Memory properties
    bool deviceLocal() const { return memProperties_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; }
    bool hostVisible() const { return memProperties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }
    bool hostCoherent() const { return memProperties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; }
    bool hostCached() const { return memProperties_ & VK_MEMORY_PROPERTY_HOST_CACHED_BIT; }

  private:
    Device* device_{nullptr};

    VkFormat format_{};
    VkExtent3D extent_{};
    VkImageUsageFlags usage_{};
    VkImage image_{VK_NULL_HANDLE};

    VkMemoryRequirements memRequirements_{};
    VkMemoryPropertyFlags memProperties_{};
    VkDeviceMemory memory_{VK_NULL_HANDLE};

    bool initialized_{false};

    bool allocateImageMemory()
    {
        vkGetImageMemoryRequirements(device_->getHandle(), image_, &memRequirements_);

        const uint32_t memIndex = utils::findMemoryType(
            device_->getPhysicalDevice(),
            MemFlagsType::requiredFlags,
            MemFlagsType::preferredFlags,
            MemFlagsType::undesiredFlags,
            memRequirements_);

        if(memIndex == ~uint32_t(0))
        {
            utils::Log::Error("vkw", "Error no available memory type");
            clear();
            return false;
        }

        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = memRequirements_.size;
        allocateInfo.memoryTypeIndex = memIndex;
        VKW_INIT_CHECK_VK(vkAllocateMemory(device_->getHandle(), &allocateInfo, nullptr, &memory_));

        // Initialize properties
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(device_->getPhysicalDevice(), &memProperties);

        const auto& props = memProperties.memoryTypes[memIndex];
        memProperties_ = props.propertyFlags;

        utils::Log::Debug("vkw", "Image memory created");
        utils::Log::Debug("vkw", "  deviceLocal:  %s", deviceLocal() ? "True" : "False");
        utils::Log::Debug("vkw", "  hostVisible:  %s", hostVisible() ? "True" : "False");
        utils::Log::Debug("vkw", "  hostCoherent: %s", hostCoherent() ? "True" : "False");
        utils::Log::Debug("vkw", "  hostCached:   %s", hostCached() ? "True" : "False");

        VKW_INIT_CHECK_VK(vkBindImageMemory(device_->getHandle(), image_, memory_, 0));

        return true;
    }
};

using DeviceImage = Image<MemoryType::Device>;
using HostStagingImage = Image<MemoryType::HostStaging>;
using HostImage = Image<MemoryType::Host>;
using HostToDeviceImage = Image<MemoryType::TransferHostDevice>;
using DeviceToHostImage = Image<MemoryType::TransferHostDevice>;
} // namespace vkw