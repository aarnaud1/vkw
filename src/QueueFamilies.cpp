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

#include "vkWrappers/wrappers/QueueFamilies.hpp"

#define CHECK_FAMILY_INDEX(index, device, surface, flag, msg)                                      \
    index = getQueueFamilyIndex(device, surface, flag);                                            \
    if(index < 0)                                                                                  \
    {                                                                                              \
        fprintf(stderr, "Error, no queue of type %s available\n", msg);                            \
        exit(1);                                                                                   \
    }

namespace vk
{
QueueFamilies::QueueFamilies(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
    this->init(physicalDevice, surface);
}

void QueueFamilies::init(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
{
    if(!initialized_)
    {
        int32_t index;
        presentSupport_ = surface != VK_NULL_HANDLE;

        CHECK_FAMILY_INDEX(index, physicalDevice, surface, GRAPHICS, "graphics");
        graphicsQueueIndex_ = (uint32_t) index;
        queueIndices_.emplace(index);

        CHECK_FAMILY_INDEX(index, physicalDevice, surface, COMPUTE, "compute");
        computeQueueIndex_ = (uint32_t) index;
        queueIndices_.emplace(index);

        CHECK_FAMILY_INDEX(index, physicalDevice, surface, TRANSFER, "transfer");
        transferQueueIndex_ = (uint32_t) index;
        queueIndices_.emplace(index);

        if(presentSupport_)
        {
            CHECK_FAMILY_INDEX(index, physicalDevice, surface, PRESENT, "present");
            presentQueueIndex_ = (uint32_t) index;
            queueIndices_.emplace(index);
        }

        initialized_ = true;
    }
}

void QueueFamilies::clear()
{
    graphicsQueueIndex_ = 0;
    computeQueueIndex_ = 0;
    transferQueueIndex_ = 0;
    presentQueueIndex_ = 0;
    queueIndices_.clear();

    presentSupport_ = false;
    initialized_ = false;
}

QueueCreateInfoList QueueFamilies::getFamilyCreateInfo()
{
    QueueCreateInfoList ret(queueIndices_.size());
    const float queuePriority = 1.0f;
    for(const auto indice : queueIndices_)
    {
        ret[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        ret[0].pNext = nullptr;
        ret[0].flags = 0;
        ret[0].queueFamilyIndex = indice;
        ret[0].queueCount = 1;
        ret[0].pQueuePriorities = &queuePriority;
    }
    return ret;
}

int QueueFamilies::getQueueFamilyIndex(
    const VkPhysicalDevice device, const VkSurfaceKHR surface, const QueueFamilyType familyType)
{
    uint32_t familyPropertiesCount = 0;
    VkQueueFamilyProperties *familyProperties = nullptr;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyPropertiesCount, nullptr);
    familyProperties = (VkQueueFamilyProperties *) malloc(
        familyPropertiesCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &familyPropertiesCount, familyProperties);

    int ret = -1;
    if(familyType == GRAPHICS)
    {
        for(uint32_t i = 0; i < familyPropertiesCount; i++)
        {
            if(familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                ret = i;
                break;
            }
        }
    }
    else if(familyType == COMPUTE)
    {
        for(uint32_t i = 0; i < familyPropertiesCount; i++)
        {
            if(familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                ret = i;
                break;
            }
        }
    }
    else if(familyType == TRANSFER)
    {
        for(uint32_t i = 0; i < familyPropertiesCount; i++)
        {
            if(familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                ret = i;
                break;
            }
        }
    }
    else if(familyType == PRESENT)
    {
        VkBool32 presentSupport = 0;
        for(uint32_t i = 0; i < familyPropertiesCount; i++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if(presentSupport)
            {
                ret = i;
                break;
            }
        }
    }
    else
    {
        fprintf(stderr, "Error :  family type not currently supported\n");
        exit(1);
    }

    free(familyProperties);
    return ret;
}

} // namespace vk
