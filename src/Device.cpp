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

#include "vkWrappers/Device.hpp"

static const char *deviceExtensions[] = {
    /*"VK_KHR_swapchain",*/ "VK_KHR_external_memory", "VK_KHR_external_memory_fd",
    "VK_KHR_external_semaphore", "VK_KHR_external_semaphore_fd"};

namespace vk
{
Device::Device(Instance &instance)
    : instance_(instance), physicalDevice_(createPhysicalDevice()),
      queueFamilies_(physicalDevice_, instance.getSurface())
{
  if(physicalDevice_ == VK_NULL_HANDLE)
  {
    fprintf(stderr, "Error, no suitable physical device\n");
    exit(1);
  }
  vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures_);

  // Create logical device
  auto queueFamilyCreateInfo = queueFamilies_.getFamilyCreateInfo();
  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyCreateInfo.size());
  deviceCreateInfo.pQueueCreateInfos = queueFamilyCreateInfo.data();
  deviceCreateInfo.enabledExtensionCount =
      /*(instance.getSurface() == VK_NULL_HANDLE) ? 0 :*/ 4;
  deviceCreateInfo.ppEnabledExtensionNames =
      /*(instance.getSurface() == VK_NULL_HANDLE) ? nullptr :*/
      deviceExtensions;

  CHECK_VK(
      vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_),
      "Creating logical device");

  queueFamilies_.getGraphicsQueue(device_, &graphicsQueue_.getHandle());
  queueFamilies_.getComputeQueue(device_, &computeQueue_.getHandle());
  queueFamilies_.getTransferQueue(device_, &transferQueue_.getHandle());
  if(instance.getSurface() != VK_NULL_HANDLE)
  {
    queueFamilies_.getPresentQueue(device_, &presentQueue_.getHandle());
  }
}

Device::~Device() { vkDestroyDevice(device_, nullptr); }

VkPhysicalDevice Device::createPhysicalDevice()
{
  uint32_t nDevices = 0;
  vkEnumeratePhysicalDevices(instance_.getInstance(), &nDevices, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(nDevices);
  vkEnumeratePhysicalDevices(instance_.getInstance(), &nDevices, physicalDevices.data());

  for(auto device : physicalDevices)
  {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
      return device;
    }
  }

  return VK_NULL_HANDLE;
}
} // namespace vk
