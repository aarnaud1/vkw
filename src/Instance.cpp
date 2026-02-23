/*
 * Copyright (c) 2026 Adrien ARNAUD
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

#include "vkw/detail/Instance.hpp"

#include "vkw/detail/utils.hpp"

#define VOLK_IMPLEMENTATION
#include <volk.h>

namespace vkw
{
VkResult initializeVulkan()
{
    static bool initialized = false;

    VkResult res = VK_SUCCESS;
    if(!initialized)
    {
        res = volkInitialize();
        if(res == VK_SUCCESS) { initialized = true; }
    }
    return res;
}

Instance::Instance(
    const VkApplicationInfo& info, const std::vector<const char*>& layers,
    const std::vector<const char*>& extensions)
{
    VKW_CHECK_BOOL_FAIL(this->init(info, layers, extensions), "Initializing instance");
}

Instance::Instance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    VKW_CHECK_BOOL_FAIL(this->init(layers, extensions), "Initializing instance");
}

Instance::Instance(Instance&& rhs) { *this = std::move(rhs); }

Instance& Instance::operator=(Instance&& rhs)
{
    this->clear();

    std::swap(instance_, rhs.instance_);
    std::swap(initialized_, rhs.initialized_);

    return *this;
}

Instance::~Instance() { clear(); }

bool Instance::init(
    const VkApplicationInfo info, const std::vector<const char*>& layers,
    const std::vector<const char*>& extensions)
{
    VKW_ASSERT(this->initialized() == false);

    VKW_INIT_CHECK_VK(initializeVulkan());

    VKW_INIT_CHECK_BOOL(checkLayersAvailable(layers));

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.pApplicationInfo = &info;
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VKW_INIT_CHECK_VK(vkCreateInstance(&createInfo, nullptr, &instance_));

    volkLoadInstanceOnly(instance_);

    initialized_ = true;

    utils::Log::Info("wkw", "Instance created");
    return true;
}

bool Instance::init(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan engine";
    appInfo.engineVersion = VK_MAKE_VERSION(2, 0, 0);
    appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0);

    return this->init(appInfo, layers, extensions);
}

void Instance::clear()
{
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
