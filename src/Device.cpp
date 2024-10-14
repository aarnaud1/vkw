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

#include "vkWrappers/wrappers/utils.hpp"

#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>

namespace vkw
{
Device::Device(
    Instance &instance,
    const std::vector<DeviceExtension> &extensions,
    const VkPhysicalDeviceFeatures &requiredFeatures,
    const std::vector<VkPhysicalDeviceType> &requiredTypes)
{
    CHECK_BOOL_THROW(
        this->init(instance, extensions, requiredFeatures, requiredTypes), "Creating device");
}

Device::Device(Device &&cp) { *this = std::move(cp); }

Device &Device::operator=(Device &&cp)
{
    this->clear();

    std::swap(instance_, cp.instance_);

    std::swap(deviceFeatures_, cp.deviceFeatures_);
    std::swap(deviceProperties_, cp.deviceProperties_);
    std::swap(physicalDevice_, cp.physicalDevice_);

    queueFamilies_ = std::move(cp.queueFamilies_);
    std::swap(device_, cp.device_);

    std::swap(initialized_, cp.initialized_);

    std::swap(graphicsQueue_, cp.graphicsQueue_);
    std::swap(computeQueue_, cp.computeQueue_);
    std::swap(transferQueue_, cp.transferQueue_);
    std::swap(presentQueue_, cp.presentQueue_);

    return *this;
}

Device::~Device() { this->clear(); }

bool Device::init(
    Instance &instance,
    const std::vector<DeviceExtension> &extensions,
    const VkPhysicalDeviceFeatures &requiredFeatures,
    const std::vector<VkPhysicalDeviceType> &requiredTypes)
{
    if(!initialized_)
    {
        instance_ = &instance;

        VKW_INIT_CHECK_BOOL(getPhysicalDevice(requiredFeatures, requiredTypes));

        vkGetPhysicalDeviceFeatures(physicalDevice_, &deviceFeatures_);
        queueFamilies_.init(physicalDevice_, instance_->getSurface());

        // Create logical device
        auto queueFamilyCreateInfo = queueFamilies_.getFamilyCreateInfo();

        std::vector<const char *> extensionNames;
        for(auto ext : extensions)
        {
            extensionNames.emplace_back(getExtensionName(ext));
        }

        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueFamilyCreateInfo.size());
        deviceCreateInfo.pQueueCreateInfos = queueFamilyCreateInfo.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
        deviceCreateInfo.pEnabledFeatures = &requiredFeatures;

        VKW_INIT_CHECK_VK(vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &device_));

        VKW_INIT_CHECK_BOOL(checkExtensionsAvailable(extensions));

        // Load required extensions
        for(auto extName : extensions)
        {
            VKW_INIT_CHECK_BOOL(loadExtension(device_, extName));
        }

        queueFamilies_.getGraphicsQueue(device_, &graphicsQueue_);
        queueFamilies_.getComputeQueue(device_, &computeQueue_);
        queueFamilies_.getTransferQueue(device_, &transferQueue_);

        const bool presentSupported
            = (std::find(extensions.begin(), extensions.end(), SwapchainKhr) != extensions.end());
        if(presentSupported)
        {
            queueFamilies_.getPresentQueue(device_, &presentQueue_);
        }

        initialized_ = true;
    }

    utils::Log::Info("wkw", "Logical device created");
    return true;
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

bool Device::getPhysicalDevice(
    const VkPhysicalDeviceFeatures &requiredFeatures,
    const std::vector<VkPhysicalDeviceType> &requiredTypes)
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
                utils::Log::Info(
                    "vkw", "Device type : %s", string_VkPhysicalDeviceType(deviceType));

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
    const VkPhysicalDeviceFeatures &requiredFeatures,
    const VkPhysicalDeviceFeatures &deviceFeatures)
{
    static constexpr uint32_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

    const auto *reqFeaturesPtr = reinterpret_cast<const VkBool32 *>(&requiredFeatures);
    const auto *featuresPtr = reinterpret_cast<const VkBool32 *>(&deviceFeatures);

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

bool Device::checkExtensionsAvailable(const std::vector<DeviceExtension> &extensionNames)
{
    const auto availableExtensions = getDeviceExtensionProperties(physicalDevice_);
    for(const auto extensionName : extensionNames)
    {
        bool found = false;
        for(const auto &extensionProperties : availableExtensions)
        {
            if(strcmp(getExtensionName(extensionName), extensionProperties.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            return false;
        }
    }

    return true;
}
} // namespace vkw
