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

#include "vkw/detail/Device.hpp"

#include "vkw/detail/utils.hpp"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <vector>

#ifndef VMA_IMPLEMENTATION
#    define VMA_IMPLEMENTATION
#    define VMA_STATIC_VULKAN_FUNCTIONS  0
#    define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif

#ifdef __GNUC__
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wconversion"
#    pragma GCC diagnostic ignored "-Wunused-parameter"
#    pragma GCC diagnostic ignored "-Wunused-variable"
#    pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wnullability-completeness"
#endif
#include <vk_mem_alloc.h>
#ifdef __clang__
#    pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#    pragma GCC diagnostic pop
#endif

namespace vkw
{
Device::Device(
    const Instance& instance, const VkPhysicalDevice& physicalDevice,
    const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& requiredFeatures,
    const void* pCreateNext)
{
    VKW_CHECK_BOOL_FAIL(
        this->init(instance, physicalDevice, extensions, requiredFeatures, pCreateNext), "Creating device");
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
    const Instance& instance, const VkPhysicalDevice& physicalDevice,
    const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& requiredFeatures,
    const void* pCreateNext)
{
    VKW_ASSERT(this->initialized() == false);

    instance_ = &instance;

    queuePriorities_.resize(maxQueueCount);
    std::fill(queuePriorities_.begin(), queuePriorities_.end(), 1.0f);

    physicalDevice_ = physicalDevice;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);

    VkPhysicalDeviceMemoryProperties memProperties = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    deviceFeatures_ = features;
    deviceProperties_ = properties;
    memProperties_ = memProperties;

    utils::Log::Info("vkw", "Device used : %s", properties.deviceName);
    utils::Log::Info("vkw", "Device type : %s", getStringDeviceType(properties.deviceType));

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
    allocatorCreateInfo.flags = useDeviceBufferAddress_ ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
    allocatorCreateInfo.physicalDevice = physicalDevice_;
    allocatorCreateInfo.device = device_;
    allocatorCreateInfo.preferredLargeHeapBlockSize = 0; // Use default value
    allocatorCreateInfo.pAllocationCallbacks = nullptr;
    allocatorCreateInfo.pDeviceMemoryCallbacks = nullptr;
    allocatorCreateInfo.pHeapSizeLimit = nullptr;
    allocatorCreateInfo.pVulkanFunctions = &vmaVkFunctions;
    allocatorCreateInfo.instance = instance_->getHandle();
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.pTypeExternalMemoryHandleTypes = nullptr;
    VKW_INIT_CHECK_VK(vmaCreateAllocator(&allocatorCreateInfo, &memAllocator_));

    initialized_ = true;

    utils::Log::Info("wkw", "Logical device created");
    return true;
}

void Device::clear()
{
    vmaDestroyAllocator(memAllocator_);
    memAllocator_ = VK_NULL_HANDLE;

    if(physicalDevice_ != VK_NULL_HANDLE) { vk().vkDestroyDevice(device_, nullptr); }

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
        if((queue.flags() & requiredFlags) == requiredFlags) { ret.emplace_back(queue); }
    }

    return ret;
}

std::vector<Queue> Device::getPresentQueues(const Surface& surface) const
{
    std::vector<Queue> ret = {};

    for(const auto& queue : deviceQueues_)
    {
        if(queue.supportsPresent(surface.getHandle())) { ret.emplace_back(queue); }
    }

    return ret;
}

VkPhysicalDeviceMemoryProperties Device::getMemoryProperties() const
{
    VKW_ASSERT(this->initialized() != false);

    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &properties);

    return properties;
}

std::vector<MemoryBudget> Device::getMemoryBudget() const
{
    VKW_ASSERT(this->initialized() != false);

    auto properties = getMemoryProperties();
    std::vector<VmaBudget> memBudgets{};

    memBudgets.resize(properties.memoryHeapCount);
    vmaGetHeapBudgets(memAllocator_, memBudgets.data());

    std::vector<MemoryBudget> ret;
    for(size_t i = 0; i < properties.memoryHeapCount; ++i)
    {
        ret.push_back({memBudgets[i].budget, memBudgets[i].usage});
    }
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
        if(queueFlags & VK_QUEUE_GRAPHICS_BIT) { flags |= uint32_t(QueueUsageBits::Graphics); }
        if(queueFlags & VK_QUEUE_COMPUTE_BIT) { flags |= uint32_t(QueueUsageBits::Compute); }
        if(queueFlags & VK_QUEUE_TRANSFER_BIT) { flags |= uint32_t(QueueUsageBits::Transfer); }
        if(queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) { flags |= uint32_t(QueueUsageBits::SparseBinding); }
        if(queueFlags & VK_QUEUE_PROTECTED_BIT) { flags |= uint32_t(QueueUsageBits::Protected); }
        if(queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) { flags |= uint32_t(QueueUsageBits::VideoDecode); }
        if(queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) { flags |= uint32_t(QueueUsageBits::VideoEncode); }

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
                useDeviceBufferAddress_ = reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures*>(next)
                                              ->bufferDeviceAddress;
                break;
            default:
                break;
        }
        next = next->pNext;
    }
}

bool Device::validateFeatures(
    const VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& curFeature)
{
    static constexpr size_t arraySize = sizeof(VkPhysicalDeviceFeatures) / sizeof(uint32_t);

    VkPhysicalDeviceFeatures2 queryFeature = {};
    queryFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    queryFeature.pNext = nullptr;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &queryFeature);

    const auto* curFeaturePtr = reinterpret_cast<const uint32_t*>(&curFeature);
    const auto* featuresPtr = reinterpret_cast<const uint32_t*>(&queryFeature);
    for(size_t i = 0; i < arraySize; ++i)
    {
        if((curFeaturePtr[i] == VK_TRUE) && (featuresPtr[i] == VK_FALSE)) { return false; }
    }

    return true;
}

bool Device::validateFeatures(
    const VkPhysicalDevice physicalDevice, const VkBaseOutStructure* curFeature, const size_t structureSize)
{
    static constexpr size_t boolOffset = sizeof(VkBaseOutStructure);

    VKW_CHECK_BOOL_RETURN_FALSE(structureSize > boolOffset);

    const size_t arraySize = (structureSize - boolOffset) / sizeof(uint32_t);
    const auto sType = curFeature->sType;

    std::vector<uint8_t> queryFeatureNextData;
    queryFeatureNextData.resize(structureSize);

    auto* queryFeatureNextPtr = reinterpret_cast<VkPhysicalDeviceFeatures2*>(queryFeatureNextData.data());
    memset(queryFeatureNextPtr, 0, structureSize);
    queryFeatureNextPtr->sType = sType;
    queryFeatureNextPtr->pNext = nullptr;

    VkPhysicalDeviceFeatures2 queryFeature = {};
    queryFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    queryFeature.pNext = queryFeatureNextPtr;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &queryFeature);

    const auto* curFeaturePtr = reinterpret_cast<const uint8_t*>(curFeature) + boolOffset;
    const auto* featuresPtr = reinterpret_cast<const uint8_t*>(queryFeatureNextPtr) + boolOffset;
    for(size_t i = 0; i < arraySize; ++i)
    {
        if((reinterpret_cast<const uint32_t*>(curFeaturePtr)[i] == VK_TRUE)
           && (reinterpret_cast<const uint32_t*>(featuresPtr)[i] == VK_FALSE))
        {
            return false;
        }
    }

    return true;
}

bool Device::checkExtensions(
    const VkPhysicalDevice physicalDevice, const std::vector<const char*>& requiredExtensions)
{
    auto extensionSupported
        = [](const char* extName, const std::vector<VkExtensionProperties>& supportedExts) {
              for(const auto& ext : supportedExts)
              {
                  if(strcmp(extName, ext.extensionName) == 0) { return true; }
              }
              return false;
          };

    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

    std::vector<VkExtensionProperties> supportedExtensions;
    supportedExtensions.resize(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(
        physicalDevice, nullptr, &deviceExtensionCount, supportedExtensions.data());

    for(const auto* extName : requiredExtensions)
    {
        if(!extensionSupported(extName, supportedExtensions)) { return false; }
    }
    return true;
}
} // namespace vkw
