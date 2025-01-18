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

#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/MemoryCommon.hpp"
#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
template <typename T, MemoryType memType>
class Buffer
{
  public:
    using value_type = T;
    using MemFlagsType = MemoryFlags<memType>;

    Buffer() {}
    explicit Buffer(
        Device& device,
        const VkBufferUsageFlags usage,
        const size_t size,
        const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        void* pCreateNext = nullptr)
    {
        VKW_CHECK_BOOL_THROW(
            this->init(device, usage, size, sharingMode, pCreateNext), "Error creating buffer");
    }

    explicit Buffer(Device& device, const VkBufferCreateInfo& createInfo)
    {
        VKW_CHECK_BOOL_THROW(this->init(device, createInfo), "Error creating buffer");
    }

    Buffer(const Buffer&) = delete;
    Buffer(Buffer&& rhs) { *this = std::move(rhs); }

    Buffer& operator=(const Buffer&) = delete;
    Buffer& operator=(Buffer&& rhs)
    {
        this->clear();

        std::swap(device_, rhs.device_);
        std::swap(buffer_, rhs.buffer_);
        std::swap(usage_, rhs.usage_);
        std::swap(size_, rhs.size_);

        std::swap(allocInfo_, rhs.allocInfo_);
        std::swap(memAllocation_, rhs.memAllocation_);

        std::swap(hostPtr_, rhs.hostPtr_);

        std::swap(initialized_, rhs.initialized_);

        return *this;
    }

    ~Buffer() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    bool init(
        Device& device,
        const VkBufferUsageFlags usage,
        const size_t size,
        const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        void* pCreateNext = nullptr)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->size_ = size;
            this->usage_ = usage;

            VkBufferCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;
            createInfo.usage = usage_;
            createInfo.size = size_ * sizeof(T);
            createInfo.sharingMode = sharingMode;
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
            VKW_INIT_CHECK_VK(vmaCreateBuffer(
                device_->allocator(),
                &createInfo,
                &allocationCreateInfo,
                &buffer_,
                &memAllocation_,
                &allocInfo_));
            hostPtr_ = reinterpret_cast<T*>(allocInfo_.pMappedData);

            utils::Log::Debug("vkw", "Buffer created");
            utils::Log::Debug("vkw", "  deviceLocal:  %s", deviceLocal() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostVisible:  %s", hostVisible() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostCoherent: %s", hostCoherent() ? "True" : "False");
            utils::Log::Debug("vkw", "  hostCached:   %s", hostCached() ? "True" : "False");

            initialized_ = true;
        }
        return true;
    }

    bool init(Device& device, const VkBufferCreateInfo& createInfo)
    {
        if(!initialized_)
        {
            this->device_ = &device;
            this->size_ = createInfo.size / sizeof(T);
            this->usage_ = createInfo.usage;

            VmaAllocationCreateInfo allocationCreateInfo = {};
            allocationCreateInfo.flags = MemFlagsType::allocationFlags;
            allocationCreateInfo.usage = MemFlagsType::usage;
            allocationCreateInfo.requiredFlags = MemFlagsType::requiredFlags;
            allocationCreateInfo.preferredFlags = MemFlagsType::preferredFlags;
            allocationCreateInfo.memoryTypeBits = 0;
            allocationCreateInfo.pool = VK_NULL_HANDLE;
            allocationCreateInfo.pUserData = nullptr;
            allocationCreateInfo.priority = 1.0f;
            VKW_INIT_CHECK_VK(vmaCreateBuffer(
                device_->allocator(),
                &createInfo,
                &allocationCreateInfo,
                &buffer_,
                &memAllocation_,
                &allocInfo_));
            hostPtr_ = reinterpret_cast<T*>(allocInfo_.pMappedData);

            utils::Log::Debug("vkw", "Buffer created");
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
        if(buffer_ != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(device_->allocator(), buffer_, memAllocation_);
            buffer_ = VK_NULL_HANDLE;
            memAllocation_ = VK_NULL_HANDLE;
        }

        size_ = 0;
        usage_ = {};
        allocInfo_ = {};

        initialized_ = false;
        device_ = nullptr;
    }

    size_t size() const { return size_; }
    size_t sizeBytes() const { return size_ * sizeof(T); }

    VkBufferUsageFlags getUsage() const { return usage_; }
    VkBuffer getHandle() const { return buffer_; }

    VkDescriptorBufferInfo getFullSizeInfo() const { return {buffer_, 0, sizeBytes()}; }
    VkDescriptorBufferInfo getDescriptorInfo(const size_t offset, const size_t size) const
    {
        return {buffer_, offset * sizeof(T), size * sizeof(T)};
    }

    void mapMemory()
    {
        static_assert(
            memType == MemoryType::Host, "Manual mapping only necessary with Host buffer type");
        VKW_CHECK_VK_THROW(
            vmaMapMemory(device_->allocator(), memAllocation_, &hostPtr_),
            "Error mapping buffer memory");
    }

    void unmapMemory()
    {
        static_assert(
            memType == MemoryType::Host, "Manual unmapping only necessary with Host buffer type");
        VKW_CHECK_VK_THROW(
            vmaUnmapMemory(device_->allocator(), memAllocation_), "Error unmapping memory");
        hostPtr_ = nullptr;
    }

    // Accessors
    inline T* data() noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_;
    }
    inline const T* data() const noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_;
    }

    inline T& operator[](const size_t i) noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_[i];
    }
    inline const T& operator[](const size_t i) const noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_[i];
    }

    inline operator T*() noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_;
    }
    inline operator const T*() const noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_;
    }

    inline T* begin() noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_;
    }
    inline const T* begin() const noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_;
    }

    inline T* end() noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_ + size_;
    }
    inline const T* end() const noexcept
    {
        static_assert(
            memType == MemoryType::Host || memType == MemoryType::HostStaging,
            "Accessors require random accessed buffer type");
        return hostPtr_ + size_;
    }

    // Copy operations
    void copyFromHost(const void* src, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");
        VKW_CHECK_VK_THROW(
            vmaCopyMemoryToAllocation(
                device_->allocator(), src, memAllocation_, 0, count * sizeof(T)),
            "Error copying from host to allocation");
    }
    void copyFromHost(const void* src, const size_t offset, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");
        VKW_CHECK_VK_THROW(
            vmaCopyMemoryToAllocation(
                device_->allocator(), src, memAllocation_, offset, count * sizeof(T)),
            "Error copying from host to allocation");
    }

    void copyToHost(void* dst, const size_t count)
    {
        static_assert(MemFlagsType::hostVisible, "copyToHost() only implemented for host buffers");
        memcpy(dst, hostPtr_, count * sizeof(T));
        VKW_CHECK_VK_THROW(
            vmaCopyAllocationToMemory(
                device_->allocator(), memAllocation_, 0, dst, count * sizeof(T)),
            "Error copying from allocation to host");
    }
    void copyToHost(void* dst, const size_t offset, const size_t count)
    {
        static_assert(MemFlagsType::hostVisible, "copyToHost() only implemented for host buffers");
        VKW_CHECK_VK_THROW(
            vmaCopyAllocationToMemory(
                device_->allocator(), memAllocation_, offset, dst, count * sizeof(T)),
            "Error copying from allocation to host");
    }

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

    size_t size_{0};
    VkBufferUsageFlags usage_{};
    VkBuffer buffer_{VK_NULL_HANDLE};

    VmaAllocationInfo allocInfo_{};
    VmaAllocation memAllocation_{VK_NULL_HANDLE};

    T* hostPtr_{nullptr};

    bool initialized_{false};
};

template <typename T>
using DeviceBuffer = Buffer<T, MemoryType::Device>;

template <typename T>
using HostStagingBuffer = Buffer<T, MemoryType::HostStaging>;

template <typename T>
using HostBuffer = Buffer<T, MemoryType::Host>;

template <typename T>
using HostToDeviceBuffer = Buffer<T, MemoryType::TransferHostDevice>;

template <typename T>
using DeviceToHostBuffer = Buffer<T, MemoryType::TransferHostDevice>;
} // namespace vkw