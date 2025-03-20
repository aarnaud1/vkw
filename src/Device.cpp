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

#include "vkWrappers/wrappers/Device.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <vector>

#ifndef VMA_IMPLEMENTATION
#    define VMA_IMPLEMENTATION
#    define VMA_STATIC_VULKAN_FUNCTIONS  0
#    define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>

namespace vkw
{
Device::Device(
    Instance& instance,
    const std::vector<const char*>& extensions,
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const std::vector<VkPhysicalDeviceType>& requiredTypes,
    const void* pCreateExt)
{
    VKW_CHECK_BOOL_THROW(
        this->init(instance, extensions, requiredFeatures, requiredTypes, pCreateExt),
        "Creating device");
}

Device::Device(Device&& rhs) { *this = std::move(rhs); }

Device& Device::operator=(Device&& rhs)
{
    this->clear();

    std::swap(instance_, rhs.instance_);

    std::swap(deviceFeatures_, rhs.deviceFeatures_);
    std::swap(deviceProperties_, rhs.deviceProperties_);
    std::swap(physicalDevice_, rhs.physicalDevice_);
    std::swap(memProperties_, rhs.memProperties_);

    std::swap(memAllocator_, rhs.memAllocator_);

    std::swap(device_, rhs.device_);

    std::swap(useDeviceBufferAddress_, rhs.useDeviceBufferAddress_);

    std::swap(initialized_, rhs.initialized_);

    return *this;
}

Device::~Device() { this->clear(); }

bool Device::init(
    Instance& instance,
    const std::vector<const char*>& extensions,
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const std::vector<VkPhysicalDeviceType>& requiredTypes,
    const void* pCreateNext)
{
    if(!initialized_)
    {
        instance_ = &instance;

        queuePriorities_.resize(maxQueueCount);
        std::fill(queuePriorities_.begin(), queuePriorities_.end(), 1.0f);

        VKW_INIT_CHECK_BOOL(getPhysicalDevice(requiredFeatures, requiredTypes));

        vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures_);
        vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties_);

        // Create logical device
        auto queueCreateInfoList = getAvailableQueuesInfo();

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = pCreateNext;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoList.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
        deviceCreateInfo.pEnabledFeatures = &requiredFeatures;
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
        VKW_INIT_CHECK_VK(vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_));
        volkLoadDeviceTable(&vkDeviceTable_, device_);

        // Get queue handles
        allocateQueues();

        validateAdditionalFeatures(reinterpret_cast<const VkBaseOutStructure*>(pCreateNext));

        VmaVulkanFunctions vmaVkFunctions = {};
        vmaVkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaVkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

        // Create memory allocator
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.flags
            = useDeviceBufferAddress_ ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
        allocatorCreateInfo.physicalDevice = physicalDevice_;
        allocatorCreateInfo.device = device_;
        allocatorCreateInfo.preferredLargeHeapBlockSize = 0; // Use default value
        allocatorCreateInfo.pAllocationCallbacks = nullptr;
        allocatorCreateInfo.pDeviceMemoryCallbacks = nullptr;
        allocatorCreateInfo.pHeapSizeLimit = nullptr;
        allocatorCreateInfo.pVulkanFunctions = &vmaVkFunctions;
        allocatorCreateInfo.instance = instance_->getHandle();
#ifdef __ANDROID__
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
#else
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
#endif
        allocatorCreateInfo.pTypeExternalMemoryHandleTypes = nullptr;
        VKW_INIT_CHECK_VK(vmaCreateAllocator(&allocatorCreateInfo, &memAllocator_));

        initialized_ = true;
    }

    utils::Log::Info("wkw", "Logical device created");
    return true;
}

void Device::clear()
{
    vmaDestroyAllocator(memAllocator_);
    memAllocator_ = VK_NULL_HANDLE;

    if(physicalDevice_ != VK_NULL_HANDLE)
    {
        vk().vkDestroyDevice(device_, nullptr);
    }

    instance_ = nullptr;

    deviceFeatures_ = {};
    deviceProperties_ = {};
    physicalDevice_ = VK_NULL_HANDLE;

    deviceQueues_.clear();
    device_ = VK_NULL_HANDLE;

    initialized_ = false;
}

std::vector<Queue> Device::getQueues(const QueueUsageFlags requiredFlags) const
{
    std::vector<Queue> ret = {};

    for(const auto& queue : deviceQueues_)
    {
        if((queue.flags() & requiredFlags) == requiredFlags)
        {
            ret.emplace_back(queue);
        }
    }

    return ret;
}

std::vector<Queue> Device::getPresentQueues(const Surface& surface) const
{
    std::vector<Queue> ret = {};

    for(const auto& queue : deviceQueues_)
    {
        if(queue.supportsPresent(surface.getHandle()))
        {
            ret.emplace_back(queue);
        }
    }

    return ret;
}

bool Device::getPhysicalDevice(
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const std::vector<VkPhysicalDeviceType>& requiredTypes)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance_->getHandle(), &physicalDeviceCount, nullptr);

    if(physicalDeviceCount == 0)
    {
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(
        instance_->getHandle(), &physicalDeviceCount, physicalDevices.data());

    for(const auto deviceType : requiredTypes)
    {
        for(const auto physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(physicalDevice, &features);

            if(properties.deviceType == deviceType
               && checkFeaturesCompatibility(requiredFeatures, features))
            {
                utils::Log::Info("vkw", "Device found : %s", properties.deviceName);
                utils::Log::Info("vkw", "Device type : %s", getStringDeviceType(deviceType));
                deviceFeatures_ = features;
                deviceProperties_ = properties;
                physicalDevice_ = physicalDevice;

                return true;
            }
        }
    }

    return false;
}

bool Device::checkFeaturesCompatibility(
    const VkPhysicalDeviceFeatures& requiredFeatures,
    const VkPhysicalDeviceFeatures& deviceFeatures)
{
    static constexpr uint32_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

    const auto* reqFeaturesPtr = reinterpret_cast<const VkBool32*>(&requiredFeatures);
    const auto* featuresPtr = reinterpret_cast<const VkBool32*>(&deviceFeatures);

    for(uint32_t i = 0; i < featureCount; ++i)
    {
        if(reqFeaturesPtr[i] == VK_TRUE && featuresPtr[i] == VK_FALSE)
        {
            return false;
        }
    }

    return true;
}

std::vector<VkExtensionProperties> Device::getDeviceExtensionProperties(
    const VkPhysicalDevice physicalDevice)
{
    uint32_t nExtensions;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nExtensions, nullptr);
    std::vector<VkExtensionProperties> ret(nExtensions);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &nExtensions, ret.data());
    return ret;
}

std::vector<VkDeviceQueueCreateInfo> Device::getAvailableQueuesInfo()
{
    deviceQueues_.clear();

    std::vector<VkDeviceQueueCreateInfo> ret{};

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> properties;
    properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, properties.data());

    for(size_t i = 0; i < properties.size(); ++i)
    {
        const auto& props = properties[i];
        const uint32_t queueCount = props.queueCount;
        const VkQueueFlags queueFlags = props.queueFlags;

        QueueUsageFlags flags = 0;
        if(queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            flags |= uint32_t(QueueUsageBits::Graphics);
        }
        if(queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            flags |= uint32_t(QueueUsageBits::Compute);
        }
        if(queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            flags |= uint32_t(QueueUsageBits::Transfer);
        }
        if(queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
        {
            flags |= uint32_t(QueueUsageBits::SparseBinding);
        }
        if(queueFlags & VK_QUEUE_PROTECTED_BIT)
        {
            flags |= uint32_t(QueueUsageBits::Protected);
        }
        if(queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
        {
            flags |= uint32_t(QueueUsageBits::VideoDecode);
        }
        if(queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
        {
            flags |= uint32_t(QueueUsageBits::VideoEncode);
        }

        for(uint32_t ii = 0; ii < std::min(queueCount, maxQueueCount); ++ii)
        {
            Queue queue{vk()};
            queue.flags_ = flags;
            queue.queueFamilyIndex_ = static_cast<uint32_t>(i);
            queue.queueIndex_ = ii;
            queue.physicalDevice_ = physicalDevice_;
            deviceQueues_.emplace_back(std::move(queue));
        }

        VkDeviceQueueCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pQueuePriorities = queuePriorities_.data();
        createInfo.queueFamilyIndex = static_cast<uint32_t>(i);
        createInfo.queueCount = std::min(queueCount, maxQueueCount);
        ret.emplace_back(createInfo);
    }

    return ret;
}

void Device::allocateQueues()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> properties;
    properties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, properties.data());

    size_t index = 0;
    for(uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const auto& props = properties[i];
        const uint32_t queueCount = props.queueCount;
        for(uint32_t ii = 0; ii < std::min(queueCount, maxQueueCount); ++ii)
        {
            vk().vkGetDeviceQueue(device_, i, ii, &(deviceQueues_[index].queue_));
            index++;
        }
    }
}

void Device::validateAdditionalFeatures(const VkBaseOutStructure* pCreateNext)
{
    VkBaseOutStructure* next = const_cast<VkBaseOutStructure*>(pCreateNext);
    while(next != nullptr)
    {
        const auto structureType = next->sType;
        switch(structureType)
        {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES:
                useDeviceBufferAddress_
                    = reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures*>(next)
                          ->bufferDeviceAddress;
                break;
            default:
                break;
        }
        next = next->pNext;
    }
}
} // namespace vkw
