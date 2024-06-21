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

#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
typedef enum QueueFamilyType
{
    GRAPHICS = 0,
    COMPUTE = 1,
    TRANSFER = 2,
    PRESENT = 3
} QueueFamilyType_t;

typedef std::vector<VkDeviceQueueCreateInfo> QueueCreateInfoList;

class QueueFamilies
{
  public:
    QueueFamilies() {}
    QueueFamilies(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface);

    QueueFamilies(const QueueFamilies &) = delete;
    QueueFamilies(QueueFamilies &&cp) = default;

    QueueFamilies &operator=(const QueueFamilies &) = delete;
    QueueFamilies &operator=(QueueFamilies &&cp) = default;

    void init(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface);
    void clear();

    bool isInitialized() const { return initialized_; }

    bool presentSupported() const { return presentSupport_; }

    QueueCreateInfoList getFamilyCreateInfo();

    template <QueueFamilyType type>
    void getQueue(VkDevice device, VkQueue *queue);

    inline void getGraphicsQueue(VkDevice device, VkQueue *graphicsQueue)
    {
        vkGetDeviceQueue(device, graphicsQueueIndex_, 0, graphicsQueue);
    }

    inline void getComputeQueue(VkDevice device, VkQueue *computeQueue)
    {
        vkGetDeviceQueue(device, computeQueueIndex_, 0, computeQueue);
    }

    inline void getTransferQueue(VkDevice device, VkQueue *transferQueue)
    {
        vkGetDeviceQueue(device, transferQueueIndex_, 0, transferQueue);
    }

    inline void getPresentQueue(VkDevice device, VkQueue *presentQueue)
    {
        if(presentSupport_)
        {
            vkGetDeviceQueue(device, presentQueueIndex_, 0, presentQueue);
        }
    }

    template <QueueFamilyType type>
    uint32_t getQueueFamilyIndex();

  private:
    uint32_t graphicsQueueIndex_{0};
    uint32_t computeQueueIndex_{0};
    uint32_t transferQueueIndex_{0};
    uint32_t presentQueueIndex_{0};
    std::set<uint32_t> queueIndices_{};
    bool presentSupport_{false};
    bool initialized_{false};

    int getQueueFamilyIndex(
        const VkPhysicalDevice device,
        const VkSurfaceKHR surface,
        const QueueFamilyType familyType);
};

template <>
inline uint32_t QueueFamilies::getQueueFamilyIndex<GRAPHICS>()
{
    return graphicsQueueIndex_;
}

template <>
inline uint32_t QueueFamilies::getQueueFamilyIndex<COMPUTE>()
{
    return computeQueueIndex_;
}

template <>
inline uint32_t QueueFamilies::getQueueFamilyIndex<TRANSFER>()
{
    return transferQueueIndex_;
}

template <>
inline uint32_t QueueFamilies::getQueueFamilyIndex<PRESENT>()
{
    return presentQueueIndex_;
}

template <>
inline void QueueFamilies::getQueue<GRAPHICS>(VkDevice device, VkQueue *queue)
{
    getGraphicsQueue(device, queue);
}

template <>
inline void QueueFamilies::getQueue<COMPUTE>(VkDevice device, VkQueue *queue)
{
    getComputeQueue(device, queue);
}

template <>
inline void QueueFamilies::getQueue<TRANSFER>(VkDevice device, VkQueue *queue)
{
    getTransferQueue(device, queue);
}

template <>
inline void QueueFamilies::getQueue<PRESENT>(VkDevice device, VkQueue *queue)
{
    getPresentQueue(device, queue);
}
} // namespace vkw
