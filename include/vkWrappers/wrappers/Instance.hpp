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

#include <cstdio>
#include <cstring>
#include <vector>

#define VKW_SURFACE_USE_NONE    0
#define VKW_SURFACE_USE_GLFW    1
#define VKW_SURFACE_USE_ANDROID 2 // Not implemented yet

#ifndef VKW_SURFACE_MODE
#    define VKW_SURFACE_MODE VKW_USE_GLFW
#endif

#if(VKW_SURFACE_MODE == VKW_USE_GLFW)
#    define GLFW_INCLUDE_VULKAN
#    include <GLFW/glfw3.h>
#elif(VKW_SURFACE_MODE == VKW_SURFACE_USE_ANDROID)
// TODO : add Android support
#endif

#include "vkWrappers/wrappers/extensions/InstanceExtensions.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <vulkan/vulkan.h>

namespace vkw
{
class Instance
{
  public:
    Instance() {};
    Instance(const std::vector<const char *> layers, std::vector<InstanceExtension> &extensions);

    Instance(const Instance &) = delete;
    Instance(Instance &&);

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&cp);

    ~Instance();

    bool init(const std::vector<const char *> layers, std::vector<InstanceExtension> &extensions);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkInstance &getInstance() { return instance_; }
    const VkInstance &getInstance() const { return instance_; }

    VkSurfaceKHR &getSurface() { return surface_; }
    const VkSurfaceKHR &getSurface() const { return surface_; }

#if(VKW_SURFACE_MODE == VKW_USE_GLFW)
    bool createSurface(GLFWwindow *window);
#else
    bool createSurface(void *);
#endif

  private:
    VkInstance instance_{VK_NULL_HANDLE};
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT callback_{nullptr};
    VkDebugReportCallbackEXT reportCallback_{nullptr};

    bool initialized_{false};

    std::vector<VkExtensionProperties> getInstanceExtensionProperties();

    std::vector<VkLayerProperties> getInstanceLayerProperties();

    std::vector<VkPhysicalDevice> listAvailablePhysicalDevices(VkInstance &instance);

    bool checkLayersAvailable(const std::vector<const char *> &layerNames);

    bool checkExtensionsAvailable(const std::vector<InstanceExtension> &extensionNames);
};
} // namespace vkw
