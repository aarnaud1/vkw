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

#include "vkWrappers/wrappers/Device.hpp"

#include <cstdio>
#include <stdexcept>
#include <vector>

static std::vector<const char *> deviceExtensions
    = {"VK_KHR_swapchain",
       "VK_KHR_external_memory",
       "VK_KHR_external_memory_fd",
       "VK_KHR_external_semaphore",
       "VK_KHR_external_semaphore_fd"};

namespace vkw
{
Device::Device(Instance &instance) { this->init(instance); }

Device::Device(Device &&cp) { *this = std::move(cp); }

Device &Device::operator=(Device &&cp)
{
    this->clear();

    std::swap(instance_, cp.instance_);
    std::swap(physicalDevice_, cp.physicalDevice_);
    queueFamilies_ = std::move(cp.queueFamilies_);
    std::swap(deviceFeatures_, cp.deviceFeatures_);
    std::swap(device_, cp.device_);
    std::swap(initialized_, cp.initialized_);

    graphicsQueue_ = cp.graphicsQueue_;
    computeQueue_ = cp.computeQueue_;
    transferQueue_ = cp.transferQueue_;
    presentQueue_ = cp.presentQueue_;

    return *this;
}

Device::~Device() { this->clear(); }

void Device::init(Instance &instance)
{
    if(!initialized_)
    {
        instance_ = &instance;
        physicalDevice_ = createPhysicalDevice();
        if(!physicalDevice_)
        {
            throw std::runtime_error("Error : no suitable physical device");
        }
        vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures_);
        queueFamilies_.init(physicalDevice_, instance_->getSurface());

        // Create logical device
        auto queueFamilyCreateInfo = queueFamilies_.getFamilyCreateInfo();

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyCreateInfo.size());
        deviceCreateInfo.pQueueCreateInfos = queueFamilyCreateInfo.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

        CHECK_VK(
            vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_),
            "Creating logical device");

        queueFamilies_.getGraphicsQueue(device_, &graphicsQueue_);
        queueFamilies_.getComputeQueue(device_, &computeQueue_);
        queueFamilies_.getTransferQueue(device_, &transferQueue_);
        if(instance.getSurface() != VK_NULL_HANDLE)
        {
            queueFamilies_.getPresentQueue(device_, &presentQueue_);
        }

        initialized_ = true;
    }
}

void Device::clear()
{
    if(queueFamilies_.isInitialized())
    {
        queueFamilies_.clear();
    }

    if(physicalDevice_ != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device_, nullptr);
    }

    instance_ = nullptr;
    physicalDevice_ = VK_NULL_HANDLE;
    deviceFeatures_ = {};

    graphicsQueue_ = VK_NULL_HANDLE;
    computeQueue_ = VK_NULL_HANDLE;
    transferQueue_ = VK_NULL_HANDLE;
    presentQueue_ = VK_NULL_HANDLE;

    initialized_ = false;
}

VkPhysicalDevice Device::createPhysicalDevice()
{
    uint32_t nDevices = 0;
    vkEnumeratePhysicalDevices(instance_->getInstance(), &nDevices, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(nDevices);
    vkEnumeratePhysicalDevices(instance_->getInstance(), &nDevices, physicalDevices.data());

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
} // namespace vkw
