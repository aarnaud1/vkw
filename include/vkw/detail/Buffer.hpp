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
class BaseBuffer
{
  public:
    BaseBuffer(const BaseBuffer&) = delete;
    BaseBuffer(BaseBuffer&&) = delete;

    BaseBuffer& operator=(const BaseBuffer&) = delete;
    BaseBuffer& operator=(BaseBuffer&&) = delete;

    virtual ~BaseBuffer() {}

    virtual bool initialized() const = 0;

    virtual VkBufferUsageFlags getUsage() const = 0;
    virtual VkBuffer getHandle() const = 0;

    virtual size_t size() const = 0;
    virtual size_t sizeBytes() const = 0;
    virtual size_t stride() const = 0;

    virtual VkDescriptorBufferInfo getFullSizeInfo() const = 0;
    virtual VkDescriptorBufferInfo getDescriptorInfo(const size_t offset, const size_t size) const
        = 0;

  protected:
    BaseBuffer() = default;
};

template <typename T, MemoryType memType, VkBufferUsageFlags additionalFlags = 0>
class Buffer : public BaseBuffer
{
  public:
    using value_type = T;
    using MemFlagsType = MemoryFlags<memType>;

    Buffer() {}
    explicit Buffer(
        const Device& device,
        const VkBufferUsageFlags usage,
        const size_t size,
        const VkDeviceSize alignment = 0,
        const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {},
        void* pCreateNext = nullptr)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(
                device, usage, size, alignment, sharingMode, queueFamilyIndices, pCreateNext),
            "Error creating buffer");
    }

    explicit Buffer(
        const Device& device,
        const VkBufferCreateInfo& createInfo,
        const VkDeviceSize alignment = 0)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, createInfo, alignment), "Error creating buffer");
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

    bool initialized() const final override { return initialized_; }

    bool init(
        const Device& device,
        const VkBufferUsageFlags usage,
        const size_t size,
        const VkDeviceSize alignment = 0,
        const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {},
        void* pCreateNext = nullptr)
    {
        VkBufferCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = pCreateNext;
        createInfo.flags = 0;
        createInfo.usage = usage;
        createInfo.size = size * sizeof(T);
        createInfo.sharingMode = sharingMode;
        createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        createInfo.pQueueFamilyIndices = queueFamilyIndices.data();

        return init(device, createInfo, alignment);
    }

    bool init(
        const Device& device,
        const VkBufferCreateInfo& createInfo,
        const VkDeviceSize alignment = 0)
    {
        VKW_ASSERT(this->initialized() == false);

        this->device_ = &device;
        this->size_ = createInfo.size / sizeof(T);
        this->usage_ = createInfo.usage | additionalFlags;

        VkBufferCreateInfo bufferCreateInfo = createInfo;
        bufferCreateInfo.usage = this->usage_;

        VmaAllocationCreateInfo allocationCreateInfo = {};
        allocationCreateInfo.flags = MemFlagsType::allocationFlags;
        allocationCreateInfo.usage = MemFlagsType::usage;
        allocationCreateInfo.requiredFlags = MemFlagsType::requiredFlags;
        allocationCreateInfo.preferredFlags = MemFlagsType::preferredFlags;
        allocationCreateInfo.memoryTypeBits = 0;
        allocationCreateInfo.pool = VK_NULL_HANDLE;
        allocationCreateInfo.pUserData = nullptr;
        allocationCreateInfo.priority = 1.0f;
        VKW_INIT_CHECK_VK(vmaCreateBufferWithAlignment(
            device_->allocator(),
            &bufferCreateInfo,
            &allocationCreateInfo,
            alignment,
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

    size_t size() const final override { return size_; }
    size_t sizeBytes() const final override { return size_ * sizeof(T); }
    size_t stride() const final override { return sizeof(T); }

    VkBufferUsageFlags getUsage() const final override { return usage_; }
    VkBuffer getHandle() const final override { return buffer_; }

    VkDescriptorBufferInfo getFullSizeInfo() const final override
    {
        return {buffer_, 0, sizeBytes()};
    }
    VkDescriptorBufferInfo getDescriptorInfo(
        const size_t offset, const size_t size) const final override
    {
        return {buffer_, offset * sizeof(T), size * sizeof(T)};
    }

    bool mapMemory()
    {
        static_assert(
            memType == MemoryType::Host, "Manual mapping only necessary with Host buffer type");

        VKW_ASSERT(this->initialized());
        VKW_CHECK_VK_RETURN_FALSE(vmaMapMemory(
            device_->allocator(), memAllocation_, reinterpret_cast<void**>(&hostPtr_)));

        return true;
    }

    void unmapMemory()
    {
        static_assert(
            memType == MemoryType::Host, "Manual unmapping only necessary with Host buffer type");

        VKW_ASSERT(this->initialized());
        vmaUnmapMemory(device_->allocator(), memAllocation_);
        hostPtr_ = nullptr;
    }

    // Buffer address
    VkDeviceAddress deviceAddress() const
    {
        VKW_ASSERT(this->initialized());
        VKW_ASSERT(device_->bufferMemoryAddressEnabled());
        VKW_ASSERT((usage_ & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0);

        VkBufferDeviceAddressInfo addressInfo = {};
        addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addressInfo.pNext = nullptr;
        addressInfo.buffer = buffer_;
        return device_->vk().vkGetBufferDeviceAddress(device_->getHandle(), &addressInfo);
    }

    // Accessors
    inline T* data() noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_;
    }
    inline const T* data() const noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_;
    }

    inline T& operator[](const size_t i) noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_[i];
    }
    inline const T& operator[](const size_t i) const noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_[i];
    }

    inline operator T*() noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_;
    }
    inline operator const T*() const noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_;
    }

    inline T* begin() noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_;
    }
    inline const T* begin() const noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_;
    }

    inline T* end() noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_ + size_;
    }
    inline const T* end() const noexcept
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Accessors require random accessed buffer type");

        return hostPtr_ + size_;
    }

    // Copy operations
    bool copyFromHost(const void* src, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");

        VKW_ASSERT(this->initialized());
        VKW_CHECK_VK_RETURN_FALSE(vmaCopyMemoryToAllocation(
            device_->allocator(), src, memAllocation_, 0, count * sizeof(T)));
        return true;
    }
    bool copyFromHost(const void* src, const size_t offset, const size_t count)
    {
        static_assert(
            MemFlagsType::hostVisible, "copyFromHost() only implemented for host buffers");

        VKW_ASSERT(this->initialized());
        VKW_CHECK_VK_RETURN_FALSE(vmaCopyMemoryToAllocation(
            device_->allocator(), src, memAllocation_, offset, count * sizeof(T)));
        return true;
    }

    bool copyToHost(void* dst, const size_t count)
    {
        static_assert(MemFlagsType::hostVisible, "copyToHost() only implemented for host buffers");

        VKW_ASSERT(this->initialized());
        memcpy(dst, hostPtr_, count * sizeof(T));
        VKW_CHECK_VK_RETURN_FALSE(vmaCopyAllocationToMemory(
            device_->allocator(), memAllocation_, 0, dst, count * sizeof(T)));
        return true;
    }
    bool copyToHost(void* dst, const size_t offset, const size_t count)
    {
        static_assert(MemFlagsType::hostVisible, "copyToHost() only implemented for host buffers");

        VKW_ASSERT(this->initialized());
        VKW_CHECK_VK_RETURN_FALSE(vmaCopyAllocationToMemory(
            device_->allocator(), memAllocation_, offset, dst, count * sizeof(T)));
        return true;
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
    const Device* device_{nullptr};

    size_t size_{0};
    VkBufferUsageFlags usage_{};
    VkBuffer buffer_{VK_NULL_HANDLE};

    VmaAllocationInfo allocInfo_{};
    VmaAllocation memAllocation_{VK_NULL_HANDLE};

    T* hostPtr_{nullptr};

    bool initialized_{false};
};

// -------------------------------------------------------------------------------------------------

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using DeviceBuffer = Buffer<T, MemoryType::Device, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostBuffer = Buffer<T, MemoryType::Host, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostStagingBuffer = Buffer<T, MemoryType::HostStaging, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostDeviceBuffer = Buffer<T, MemoryType::HostDevice, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostToDeviceBuffer = Buffer<T, MemoryType::TransferHostDevice, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using DeviceToHostBuffer = Buffer<T, MemoryType::TransferDeviceHost, additionalFlags>;
} // namespace vkw