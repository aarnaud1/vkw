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

#include <set>
#include <cstdlib>
#include <cstdio>
#include <vector>

#include <vulkan/vulkan.h>

#include "vkWrappers/wrappers/utils.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/QueueFamilies.hpp"

namespace vk
{
class Device
{
  public:
    Device(Instance &instance);

    Device(const Device &) = delete;
    Device(Device &&) = delete;

    Device &operator=(const Device &) = delete;
    Device &operator=(Device &&) = delete;

    ~Device();

    QueueFamilies &getQueueFamilies() { return queueFamilies_; }

    VkDevice &getHandle() { return device_; }

    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }

    VkQueue getQueue(const QueueFamilyType type)
    {
        if(type == QueueFamilyType::COMPUTE)
        {
            return computeQueue_;
        }

        if(type == QueueFamilyType::GRAPHICS)
        {
            return graphicsQueue_;
        }

        if(type == QueueFamilyType::PRESENT)
        {
            return presentQueue_;
        }

        if(type == QueueFamilyType::TRANSFER)
        {
            return transferQueue_;
        }

        return VK_NULL_HANDLE;
    }

  private:
    Instance &instance_;
    VkPhysicalDevice physicalDevice_;
    QueueFamilies queueFamilies_;
    VkPhysicalDeviceFeatures deviceFeatures_{};
    VkDevice device_{VK_NULL_HANDLE};

    VkQueue graphicsQueue_{VK_NULL_HANDLE};
    VkQueue computeQueue_{VK_NULL_HANDLE};
    VkQueue transferQueue_{VK_NULL_HANDLE};
    VkQueue presentQueue_{VK_NULL_HANDLE};

    VkPhysicalDevice createPhysicalDevice();
};
} // namespace vk
