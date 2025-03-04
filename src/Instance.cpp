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

#include "vkWrappers/wrappers/Instance.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#define VOLK_IMPLEMENTATION
#include <volk.h>

static VkResult initializeVulkan()
{
    static bool initialized = false;

    VkResult res = VK_SUCCESS;
    if(!initialized)
    {
        res = volkInitialize();
        if(res == VK_SUCCESS)
        {
            initialized = true;
        }
    }
    return res;
}

namespace vkw
{
Instance::Instance(
    const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    VKW_CHECK_BOOL_THROW(this->init(layers, extensions), "Initializing instance");
}

Instance::Instance(Instance&& rhs) { *this = std::move(rhs); }

Instance& Instance::operator=(Instance&& rhs)
{
    this->clear();

    std::swap(instance_, rhs.instance_);
    std::swap(surface_, rhs.surface_);

    std::swap(initialized_, rhs.initialized_);

    return *this;
}

Instance::~Instance() { clear(); }

bool Instance::init(
    const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    VKW_INIT_CHECK_VK(initializeVulkan());

    if(!initialized_)
    {
        // Instance creation
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan engine";
        appInfo.engineVersion = VK_MAKE_VERSION(2, 0, 0);
        appInfo.apiVersion = VK_VERSION_1_4;

        VKW_INIT_CHECK_BOOL(checkLayersAvailable(layers));

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        VKW_INIT_CHECK_VK(vkCreateInstance(&createInfo, nullptr, &instance_));

        volkLoadInstanceOnly(instance_);

        initialized_ = true;
    }

    utils::Log::Info("wkw", "Instance created");
    return true;
}

void Instance::clear()
{
    if(surface_ != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    if(instance_ != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }

    initialized_ = false;
}

std::vector<VkExtensionProperties> Instance::getInstanceExtensionProperties()
{
    uint32_t nExtensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &nExtensions, nullptr);
    std::vector<VkExtensionProperties> ret(nExtensions);
    vkEnumerateInstanceExtensionProperties(nullptr, &nExtensions, ret.data());
    return ret;
}

std::vector<VkLayerProperties> Instance::getInstanceLayerProperties()
{
    uint32_t nLayers;
    vkEnumerateInstanceLayerProperties(&nLayers, nullptr);
    std::vector<VkLayerProperties> ret(nLayers);
    vkEnumerateInstanceLayerProperties(&nLayers, ret.data());
    return ret;
}

bool Instance::checkLayersAvailable(const std::vector<const char*>& layerNames)
{
    const auto availableLayers = getInstanceLayerProperties();
    for(const auto* layerName : layerNames)
    {
        bool found = false;
        for(const auto& layerProperties : availableLayers)
        {
            if(strcmp(layerName, layerProperties.layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            fprintf(stderr, "%s : not available\n", layerName);
            return false;
        }
    }

    return true;
}
} // namespace vkw
