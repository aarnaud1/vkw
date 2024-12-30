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
        std::swap(memory_, rhs.memory_);
        std::swap(allocatedBytes_, rhs.allocatedBytes_);

        std::swap(usage_, rhs.usage_);
        std::swap(size_, rhs.size_);

        std::swap(memRequirements_, rhs.memRequirements_);
        std::swap(memProperties_, rhs.memProperties_);

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
            VKW_INIT_CHECK_VK(vkCreateBuffer(device_->getHandle(), &createInfo, nullptr, &buffer_));
            VKW_INIT_CHECK_BOOL(allocateBufferMemory());

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

            VKW_INIT_CHECK_VK(vkCreateBuffer(device_->getHandle(), &createInfo, nullptr, &buffer_));
            VKW_INIT_CHECK_BOOL(allocateBufferMemory());

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        if(hostPtr_ != nullptr)
        {
            vkUnmapMemory(device_->getHandle(), memory_);
        }

        VKW_DELETE_VK(Buffer, buffer_);
        VKW_FREE_VK(Memory, memory_);

        size_ = 0;
        usage_ = {};
        memRequirements_ = {};
        memProperties_ = {};
        allocatedBytes_ = 0;

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

    // Accessors
    inline T* data() noexcept
    {
        static_assert(MemFlagsType::hostVisible, "data() only implemented for host buffers");
        return hostPtr_;
    }
    inline const T* data() const noexcept
    {
        static_assert(MemFlagsType::hostVisible, "data() only implemented for host buffers");
        return hostPtr_;
    }

    inline T& operator[](const size_t i) noexcept
    {
        static_assert(
            MemFlagsType::hostVisible, "braces operator only implemented for host buffers");
        return hostPtr_[i];
    }
    inline const T& operator[](const size_t i) const noexcept
    {
        static_assert(
            MemFlagsType::hostVisible, "braces operator only implemented for host buffers");
        return hostPtr_[i];
    }

    inline operator T*() noexcept
    {
        static_assert(MemFlagsType::hostVisible, "cast operator only implemented for host buffers");
        return hostPtr_;
    }
    inline operator const T*() const noexcept
    {
        static_assert(MemFlagsType::hostVisible, "cast operator only implemented for host buffers");
        return hostPtr_;
    }

    inline T* begin() noexcept
    {
        static_assert(MemFlagsType::hostVisible, "begin() only implemented for host buffers");
        return hostPtr_;
    }
    inline const T* begin() const noexcept
    {
        static_assert(MemFlagsType::hostVisible, "begin() only implemented for host buffers");
        return hostPtr_;
    }

    inline T* end() noexcept
    {
        static_assert(MemFlagsType::hostVisible, "end() only implemented for host buffers");
        return hostPtr_ + size_;
    }
    inline const T* end() const noexcept
    {
        static_assert(MemFlagsType::hostVisible, "end() only implemented for host buffers");
        return hostPtr_ + size_;
    }

    // Transfer operations
    void copyFromHost(const void* src, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");
        if(count > size())
        {
            throw std::runtime_error("Not enough space in buffer");
        }
        memcpy(hostPtr_, src, count * sizeof(T));
        if(!hostCoherent())
        {
            flushMappedMemory();
        }
    }

    void copyFromHost(const void* src, const size_t offset, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");
        if(size() < (count + offset))
        {
            throw std::runtime_error("Not enough space in buffer");
        }
        memcpy(hostPtr_ + offset * sizeof(T), src, count * sizeof(T));
        if(!hostCoherent())
        {
            flushMappedMemory();
        }
    }

    void copyToHost(void* dst, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");
        if(count > size())
        {
            throw std::runtime_error("Not enough space in buffer");
        }
        if(!hostCoherent())
        {
            invalidateMappedMemory();
        }
        memcpy(dst, hostPtr_, count * sizeof(T));
    }

    void copyToHost(void* dst, const size_t offset, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");
        if(size() <= (count + offset))
        {
            throw std::runtime_error("Not enough space in buffer");
        }
        if(!hostCoherent())
        {
            invalidateMappedMemory();
        }
        memcpy(dst, hostPtr_ + offset * sizeof(T), count * sizeof(T));
    }

    void flushMappedMemory()
    {
        static_assert(memType != MemoryType::Device, "Device memory cannot be mapped");
        if(!hostCoherent())
        {
            VkMappedMemoryRange memoryRange = {};
            memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.pNext = nullptr;
            memoryRange.offset = 0;
            memoryRange.size = VK_WHOLE_SIZE;
            memoryRange.memory = memory_;

            VKW_CHECK_VK_THROW(
                vkFlushMappedMemoryRanges(device_->getHandle(), 1, &memoryRange),
                "Flushing mapped memory");
        }
    }

    void invalidateMappedMemory()
    {
        static_assert(memType != MemoryType::Device, "Device memory cannot be mapped");
        if(!hostCoherent())
        {
            VkMappedMemoryRange memoryRange = {};
            memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memoryRange.pNext = nullptr;
            memoryRange.offset = 0;
            memoryRange.size = VK_WHOLE_SIZE;
            memoryRange.memory = memory_;

            VKW_CHECK_VK_THROW(
                vkInvalidateMappedMemoryRanges(device_->getHandle(), 1, &memoryRange),
                "Invalidating mapped memory");
        }
    }

    // Memory properties
    bool deviceLocal() const { return memProperties_ & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; }
    bool hostVisible() const { return memProperties_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }
    bool hostCoherent() const { return memProperties_ & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; }
    bool hostCached() const { return memProperties_ & VK_MEMORY_PROPERTY_HOST_CACHED_BIT; }

  private:
    Device* device_{nullptr};

    size_t size_{0};
    VkBufferUsageFlags usage_{0};
    VkBuffer buffer_{VK_NULL_HANDLE};

    size_t allocatedBytes_{0};
    VkMemoryRequirements memRequirements_{};
    VkMemoryPropertyFlags memProperties_{};
    VkDeviceMemory memory_{VK_NULL_HANDLE};

    T* hostPtr_{nullptr};

    bool initialized_{false};

    bool allocateBufferMemory()
    {
        vkGetBufferMemoryRequirements(device_->getHandle(), buffer_, &memRequirements_);

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

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device_->getPhysicalDevice(), &properties);
        const VkDeviceSize nonCoherentSize = properties.limits.nonCoherentAtomSize;
        allocatedBytes_ = utils::alignedSize(memRequirements_.size, nonCoherentSize);

        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = allocatedBytes_;
        allocateInfo.memoryTypeIndex = memIndex;
        VKW_INIT_CHECK_VK(vkAllocateMemory(device_->getHandle(), &allocateInfo, nullptr, &memory_));

        // Initialize properties
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(device_->getPhysicalDevice(), &memProperties);

        const auto& props = memProperties.memoryTypes[memIndex];
        memProperties_ = props.propertyFlags;

        utils::Log::Debug("vkw", "Buffer memory created");
        utils::Log::Debug("vkw", "  deviceLocal:  %s", deviceLocal() ? "True" : "False");
        utils::Log::Debug("vkw", "  hostVisible:  %s", hostVisible() ? "True" : "False");
        utils::Log::Debug("vkw", "  hostCoherent: %s", hostCoherent() ? "True" : "False");
        utils::Log::Debug("vkw", "  hostCached:   %s", hostCached() ? "True" : "False");

        if constexpr(memType != MemoryType::Device)
        {
            VKW_INIT_CHECK_VK(vkMapMemory(
                device_->getHandle(),
                memory_,
                0,
                VK_WHOLE_SIZE,
                0,
                reinterpret_cast<void**>(&hostPtr_)));
        }

        VKW_INIT_CHECK_VK(vkBindBufferMemory(device_->getHandle(), buffer_, memory_, 0));

        return true;
    }
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