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

#include "vkWrappers/wrappers/Instance.hpp"

#include "vkWrappers/wrappers/extensions/InstanceExtensions.hpp"
#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
Instance::Instance(
    const std::vector<const char *> &layers, const std::vector<InstanceExtension> &extensions)
{
    CHECK_BOOL_THROW(this->init(layers, extensions), "Initializing instance");
}

Instance::Instance(Instance &&cp) { *this = std::move(cp); }

Instance &Instance::operator=(Instance &&cp)
{
    this->clear();

    std::swap(instance_, cp.instance_);
    std::swap(surface_, cp.surface_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

Instance::~Instance() { clear(); }

bool Instance::init(
    const std::vector<const char *> &layers, const std::vector<InstanceExtension> &extensions)
{
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
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VKW_INIT_CHECK_BOOL(checkLayersAvailable(layers));
        VKW_INIT_CHECK_BOOL(checkExtensionsAvailable(extensions));

        std::vector<const char *> extensionNames;
        for(auto extName : extensions)
        {
            extensionNames.emplace_back(getExtensionName(extName));
        }

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensionNames.data();

        VKW_INIT_CHECK_VK(vkCreateInstance(&createInfo, nullptr, &instance_));

        // Load required extensions
        for(auto extName : extensions)
        {
            VKW_INIT_CHECK_BOOL(loadExtension(instance_, extName));
        }

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

#if(VKW_SURFACE_MODE == VKW_USE_GLFW)
bool Instance::createSurface(GLFWwindow *window)
{
    if(instance_ == nullptr)
    {
        return false;
    }

    if(window != nullptr)
    {
        CHECK_VK_RETURN_FALSE(glfwCreateWindowSurface(instance_, window, nullptr, &surface_));
    }
    return true;
}
#else
bool Instance::createSurface(void *)
{
#    error Instance::createSurface not defined
}
#endif

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

bool Instance::checkLayersAvailable(const std::vector<const char *> &layerNames)
{
    const auto availableLayers = getInstanceLayerProperties();
    for(const auto *layerName : layerNames)
    {
        bool found = false;
        for(const auto &layerProperties : availableLayers)
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

bool Instance::checkExtensionsAvailable(const std::vector<InstanceExtension> &extensionNames)
{
    const auto availableExtensions = getInstanceExtensionProperties();
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
            fprintf(stderr, "%s : not available\n", getExtensionName(extensionName));
            return false;
        }
    }

    return true;
}
} // namespace vkw
