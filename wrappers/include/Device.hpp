/*
 * Copyright (C) 2022 Adrien ARNAUD
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

#include "utils.hpp"
#include "Instance.hpp"
#include "QueueFamilies.hpp"
#include "Queue.hpp"

namespace vk
{
class Device
{
public:
  Device(Instance &instance);

  ~Device();

  QueueFamilies &getQueueFamilies() { return queueFamilies_; }

  VkDevice &getHandle() { return device_; }

  VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }

  template <QueueFamilyType type>
  Queue<type> &getQueue();

private:
  Instance &instance_;
  VkPhysicalDevice physicalDevice_;
  QueueFamilies queueFamilies_;
  VkPhysicalDeviceFeatures deviceFeatures_;
  VkDevice device_;

  Queue<QueueFamilyType::GRAPHICS> graphicsQueue_;
  Queue<QueueFamilyType::COMPUTE> computeQueue_;
  Queue<QueueFamilyType::TRANSFER> transferQueue_;
  Queue<QueueFamilyType::PRESENT> presentQueue_;

  VkPhysicalDevice createPhysicalDevice();
};

template <>
inline Queue<QueueFamilyType::GRAPHICS> &
Device::getQueue<QueueFamilyType::GRAPHICS>()
{
  return graphicsQueue_;
}

template <>
inline Queue<QueueFamilyType::COMPUTE> &
Device::getQueue<QueueFamilyType::COMPUTE>()
{
  return computeQueue_;
}

template <>
inline Queue<QueueFamilyType::TRANSFER> &
Device::getQueue<QueueFamilyType::TRANSFER>()
{
  return transferQueue_;
}

template <>
inline Queue<QueueFamilyType::PRESENT> &
Device::getQueue<QueueFamilyType::PRESENT>()
{
  return presentQueue_;
}
} // namespace vk
