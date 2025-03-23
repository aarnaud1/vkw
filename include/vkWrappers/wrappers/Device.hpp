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

#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/Queue.hpp"
#include "vkWrappers/wrappers/Surface.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdlib>

// Forward declaration of VmaAllocator
struct VmaAllocator_T;
typedef struct VmaAllocator_T* VmaAllocator;

namespace vkw
{
template <typename... Args>
static std::vector<VkPhysicalDevice> listSupportedDevices(
    const Instance& instance,
    const std::vector<const char*>& requiredExtensions,
    const VkPhysicalDeviceFeatures& requiredFeatures,
    Args&&... additionalFeatures);
static std::vector<VkPhysicalDevice> listSupportedDevices(
    const Instance& instance,
    const std::vector<const char*>& requiredExtensions,
    const VkPhysicalDeviceFeatures& requiredFeatures);

class Device
{
  public:
    Device() {}

    Device(
        Instance& instance,
        const VkPhysicalDevice& physicalDevice,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const void* pCreateNext = nullptr);

    Device(const Device&) = delete;
    Device(Device&& cp);

    Device& operator=(const Device&) = delete;
    Device& operator=(Device&& cp);

    ~Device();

    bool init(
        Instance& instance,
        const VkPhysicalDevice& physicalDevice,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const void* pCreateNext = nullptr);

    void clear();

    bool isInitialized() const { return initialized_; }

    std::vector<Queue> getQueues(const QueueUsageFlags requiredFlags) const;
    std::vector<Queue> getPresentQueues(const Surface& surface) const;

    inline const auto& vk() const { return vkDeviceTable_; }

    auto getHandle() const { return device_; }
    auto allocator() const { return memAllocator_; }

    auto bufferMemoryAddressEnabled() const { return useDeviceBufferAddress_; }

    VkPhysicalDeviceFeatures getFeatures() const { return deviceFeatures_; }
    VkPhysicalDeviceProperties getProperties() const { return deviceProperties_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    const auto& getMemProperties() const { return memProperties_; }

    void waitIdle() const { vk().vkDeviceWaitIdle(device_); }

  private:
    Instance* instance_{nullptr};
    VolkDeviceTable vkDeviceTable_{};

    VkPhysicalDeviceFeatures deviceFeatures_{};
    VkPhysicalDeviceProperties deviceProperties_{};
    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkPhysicalDeviceMemoryProperties memProperties_{};

    VmaAllocator memAllocator_{VK_NULL_HANDLE};

    static constexpr uint32_t maxQueueCount = 32;
    std::vector<float> queuePriorities_;

    std::vector<Queue> deviceQueues_{};
    VkDevice device_{VK_NULL_HANDLE};

    VkBool32 useDeviceBufferAddress_{VK_FALSE};

    bool initialized_{false};

    void allocateQueues();

    std::vector<VkDeviceQueueCreateInfo> getAvailableQueuesInfo();

    void validateAdditionalFeatures(const VkBaseOutStructure* pCreateNext);
};

static bool validateFeatures(
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
        if((curFeaturePtr[i] == VK_TRUE) && (featuresPtr[i] == VK_FALSE))
        {
            return false;
        }
    }

    return true;
}

template <typename FeatureType>
static bool validateFeatures(const VkPhysicalDevice physicalDevice, const FeatureType& curFeature)
{
    static constexpr size_t boolOffset = sizeof(VkBaseOutStructure);
    static constexpr size_t arraySize = (sizeof(FeatureType) - boolOffset) / sizeof(uint32_t);
    static_assert(
        sizeof(FeatureType) > boolOffset,
        "Error FeatureType is probably not an instance of VkBaseOutStructure");

    const auto sType = curFeature.sType;

    FeatureType queryFeatureNext = {};
    queryFeatureNext.sType = sType;
    queryFeatureNext.pNext = nullptr;

    VkPhysicalDeviceFeatures2 queryFeature = {};
    queryFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    queryFeature.pNext = &queryFeatureNext;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &queryFeature);

    const auto* curFeaturePtr = reinterpret_cast<const uint8_t*>(&curFeature) + boolOffset;
    const auto* featuresPtr = reinterpret_cast<const uint8_t*>(&queryFeatureNext) + boolOffset;
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

template <typename FeatureType, typename... Args>
static bool validateFeatures(
    const VkPhysicalDevice physicalDevice, const FeatureType& feature, Args&&... otherFeatures)
{
    if(!validateFeatures(physicalDevice, feature))
    {
        return false;
    }
    return validateFeatures(physicalDevice, std::forward<Args>(otherFeatures)...);
}

static std::vector<VkPhysicalDevice> listSupportedDevices(
    const Instance& instance,
    const std::vector<const char*>& requiredExtensions,
    const VkPhysicalDeviceFeatures& requiredFeatures)
{
    auto extensionSupported
        = [](const char* extName, const std::vector<VkExtensionProperties>& supportedExts) {
              for(const auto& ext : supportedExts)
              {
                  if(strcmp(extName, ext.extensionName) == 0)
                  {
                      return true;
                  }
              }
              return false;
          };
    auto checkExtensions = [extensionSupported](
                               const std::vector<const char*>& requiredExts,
                               const std::vector<VkExtensionProperties>& supportedExts) {
        for(const auto* extName : requiredExts)
        {
            if(!extensionSupported(extName, supportedExts))
            {
                return false;
            }
        }
        return true;
    };

    std::vector<VkPhysicalDevice> ret = {};

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, physicalDevices.data());

    for(const auto physicalDevice : physicalDevices)
    {
        // Check all extensions are supported
        uint32_t supportedExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(
            physicalDevice, nullptr, &supportedExtensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions;
        supportedExtensions.resize(supportedExtensionCount);
        vkEnumerateDeviceExtensionProperties(
            physicalDevice, nullptr, &supportedExtensionCount, supportedExtensions.data());

        if(!checkExtensions(requiredExtensions, supportedExtensions))
        {
            continue;
        }

        if(!validateFeatures(physicalDevice, requiredFeatures))
        {
            continue;
        }

        ret.push_back(physicalDevice);
    }

    return ret;
}

template <typename... Args>
static std::vector<VkPhysicalDevice> listSupportedDevices(
    const Instance& instance,
    const std::vector<const char*>& requiredExtensions,
    const VkPhysicalDeviceFeatures& requiredFeatures,
    Args&&... additionalFeatures)
{
    auto extensionSupported
        = [](const char* extName, const std::vector<VkExtensionProperties>& supportedExts) {
              for(const auto& ext : supportedExts)
              {
                  if(strcmp(extName, ext.extensionName) == 0)
                  {
                      return true;
                  }
              }
              return false;
          };
    auto checkExtensions = [extensionSupported](
                               const std::vector<const char*>& requiredExts,
                               const std::vector<VkExtensionProperties>& supportedExts) {
        for(const auto* extName : requiredExts)
        {
            if(!extensionSupported(extName, supportedExts))
            {
                return false;
            }
        }
        return true;
    };

    std::vector<VkPhysicalDevice> ret = {};

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, physicalDevices.data());

    for(const auto physicalDevice : physicalDevices)
    {
        // Check all extensions are supported
        uint32_t supportedExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(
            physicalDevice, nullptr, &supportedExtensionCount, nullptr);

        std::vector<VkExtensionProperties> supportedExtensions;
        supportedExtensions.resize(supportedExtensionCount);
        vkEnumerateDeviceExtensionProperties(
            physicalDevice, nullptr, &supportedExtensionCount, supportedExtensions.data());

        if(!checkExtensions(requiredExtensions, supportedExtensions))
        {
            continue;
        }

        if(!validateFeatures(physicalDevice, requiredFeatures))
        {
            continue;
        }

        if(!validateFeatures(physicalDevice, std::forward<Args>(additionalFeatures)...))
        {
            continue;
        }

        ret.push_back(physicalDevice);
    }

    return ret;
}
} // namespace vkw
