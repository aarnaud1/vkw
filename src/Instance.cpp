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

#include "vkWrappers/wrappers/Validation.hpp"

#define DISABLE_VALIDATION

#ifdef DISABLE_VALIDATION
static const std::vector<const char *> validationLayers = {};
#else
static const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#endif
static const std::vector<const char *> extensions
    = {"VK_EXT_debug_utils", "VK_KHR_surface", "VK_KHR_xcb_surface", "VK_EXT_debug_report"};

namespace vk
{
Instance::Instance(GLFWwindow *window) : window_(window) { this->init(window); }

Instance::Instance(Instance &&cp) { *this = std::move(cp); }

Instance &Instance::operator=(Instance &&cp)
{
    this->clear();

    std::swap(window_, cp.window_);
    std::swap(instance_, cp.instance_);
    std::swap(surface_, cp.surface_);
    std::swap(callback_, cp.callback_);
    std::swap(reportCallback_, cp.reportCallback_);
    std::swap(initialized_, cp.initialized_);

    return *this;
}

Instance::~Instance() { clear(); }

void Instance::init(GLFWwindow *window)
{
    if(!initialized_)
    {
        window_ = window;

        // Instance creation
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Test vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        if(!checkLayersAvailable(validationLayers))
        {
            fprintf(stderr, "Error : some validation layers not available\n");
            exit(1);
        }
        if(!checkExtensionsAvailable(extensions))
        {
            fprintf(stderr, "Error : some instance extensions not available\n");
            exit(1);
        }

        // Report callback
        // VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {};
        // debugReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        // debugReportCreateInfo.pNext = nullptr;
        // debugReportCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
        // debugReportCreateInfo.pfnCallback = debugReportCallback;
        // debugReportCreateInfo.pUserData = nullptr;

        // Debug callback
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        // debugCreateInfo.pNext = (VkDebugReportCallbackCreateInfoEXT *) &debugReportCreateInfo;
        debugCreateInfo.pNext = nullptr;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                          | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pUserData = nullptr;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        CHECK_VK(vkCreateInstance(&createInfo, nullptr, &instance_), "Creating instance");
        CHECK_VK(
            CreateDebugUtilsMessengerEXT(instance_, &debugCreateInfo, nullptr, &callback_),
            "Creating debug messenger");
        // CHECK_VK(
        //     CreateDebugReportCallbackEXT(instance_, &debugReportCreateInfo, nullptr,
        //     &reportCallback_), "Creating report messenger");
        if(window_ != nullptr)
        {
            CHECK_VK(
                glfwCreateWindowSurface(instance_, window_, nullptr, &surface_),
                "Creating surface");
        }

        initialized_ = true;
    }
}

void Instance::clear()
{
    if(surface_ != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    // if(reportCallback_ != nullptr)
    // {
    // DestroyDebugReportCallbackEXT(instance_, reportCallback_, nullptr);
    // reportCallback_ = nullptr;
    // }

    if(callback_ != nullptr)
    {
        DestroyDebugUtilsMessengerEXT(instance_, callback_, nullptr);
        callback_ = nullptr;
    }

    if(instance_ != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }

    window_ = nullptr;
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

std::vector<VkPhysicalDevice> Instance::listAvailablePhysicalDevices(VkInstance &instance)
{
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> ret(count);
    vkEnumeratePhysicalDevices(instance, &count, ret.data());
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

bool Instance::checkExtensionsAvailable(const std::vector<const char *> &extensionNames)
{
    const auto availableExtensions = getInstanceExtensionProperties();
    for(const auto *extensionName : extensionNames)
    {
        bool found = false;
        for(const auto &extensionProperties : availableExtensions)
        {
            if(strcmp(extensionName, extensionProperties.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            fprintf(stderr, "%s : not available\n", extensionName);
            return false;
        }
    }

    return true;
}
} // namespace vk
