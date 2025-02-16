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
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdlib>

// Forward declaration of VmaAllocator
struct VmaAllocator_T;
typedef struct VmaAllocator_T* VmaAllocator;

namespace vkw
{
class Device
{
  public:
    Device() {}
    Device(
        Instance& instance,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const std::vector<VkPhysicalDeviceType>& requiredTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU},
        const void* pCreateNext = nullptr);

    Device(const Device&) = delete;
    Device(Device&& cp);

    Device& operator=(const Device&) = delete;
    Device& operator=(Device&& cp);

    ~Device();

    bool init(
        Instance& instance,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const std::vector<VkPhysicalDeviceType>& requiredTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU},
        const void* pCreateNext = nullptr);

    void clear();

    bool isInitialized() const { return initialized_; }

    std::vector<Queue> getQueues(const QueueUsageFlags requiredFlags) const;

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

    bool getPhysicalDevice(
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const std::vector<VkPhysicalDeviceType>& requiredTypes);

    bool checkFeaturesCompatibility(
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const VkPhysicalDeviceFeatures& deviceFeatures);

    std::vector<VkExtensionProperties> getDeviceExtensionProperties(
        const VkPhysicalDevice physicalDevice);

    void allocateQueues();

    std::vector<VkDeviceQueueCreateInfo> getAvailableQueuesInfo();

    void validateAdditionalFeatures(const VkBaseOutStructure* pCreateNext);
};
} // namespace vkw
