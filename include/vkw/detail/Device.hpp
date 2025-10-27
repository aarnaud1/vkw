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
#include "vkw/detail/Instance.hpp"
#include "vkw/detail/Queue.hpp"
#include "vkw/detail/Surface.hpp"
#include "vkw/detail/utils.hpp"

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
        const Instance& instance, const VkPhysicalDevice& physicalDevice,
        const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& requiredFeatures,
        const void* pCreateNext = nullptr);

    Device(const Device&) = delete;
    Device(Device&& cp);

    Device& operator=(const Device&) = delete;
    Device& operator=(Device&& cp);

    ~Device();

    bool init(
        const Instance& instance, const VkPhysicalDevice& physicalDevice,
        const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& requiredFeatures,
        const void* pCreateNext = nullptr);

    void clear();

    bool initialized() const { return initialized_; }

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

    static std::vector<VkPhysicalDevice> listSupportedDevices(
        const Instance& instance, const std::vector<const char*>& requiredExtensions,
        const VkPhysicalDeviceFeatures& requiredFeatures)
    {
        std::vector<VkPhysicalDevice> ret = {};

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices;
        physicalDevices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, physicalDevices.data());

        for(const auto physicalDevice : physicalDevices)
        {
            if(!checkExtensions(physicalDevice, requiredExtensions)) { continue; }

            if(!validateFeatures(physicalDevice, requiredFeatures)) { continue; }

            ret.push_back(physicalDevice);
        }

        return ret;
    }

    template <typename... Args>
    static std::vector<VkPhysicalDevice> listSupportedDevices(
        const Instance& instance, const std::vector<const char*>& requiredExtensions,
        const VkPhysicalDeviceFeatures& requiredFeatures, Args&&... additionalFeatures)
    {
        std::vector<VkPhysicalDevice> ret = {};

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices;
        physicalDevices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, physicalDevices.data());

        for(const auto physicalDevice : physicalDevices)
        {
            if(!checkExtensions(physicalDevice, requiredExtensions)) { continue; }

            if(!validateFeatures(physicalDevice, requiredFeatures)) { continue; }

            if(!validateFeatures(physicalDevice, std::forward<Args>(additionalFeatures)...)) { continue; }

            ret.push_back(physicalDevice);
        }

        return ret;
    }

  private:
    const Instance* instance_{nullptr};
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

    static bool validateFeatures(
        const VkPhysicalDevice physicalDevice, const VkPhysicalDeviceFeatures& curFeature);
    static bool validateFeatures(
        const VkPhysicalDevice physicalDevice, const VkBaseOutStructure* curFeature,
        const size_t structureSize);

    template <typename FeatureType>
    static inline bool validateFeatures(const VkPhysicalDevice physicalDevice, const FeatureType& curFeature)
    {
        return validateFeatures(
            physicalDevice, reinterpret_cast<const VkBaseOutStructure*>(&curFeature), sizeof(FeatureType));
    }

    template <typename FeatureType, typename... Args>
    static bool validateFeatures(
        const VkPhysicalDevice physicalDevice, const FeatureType& feature, Args&&... otherFeatures)
    {
        if(!validateFeatures(physicalDevice, feature)) { return false; }
        return validateFeatures(physicalDevice, std::forward<Args>(otherFeatures)...);
    }

    static bool checkExtensions(
        const VkPhysicalDevice physicalDevice, const std::vector<const char*>& requiredExtensions);
};
} // namespace vkw
