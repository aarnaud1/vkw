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

#ifndef GLFW_INCLUDE_VULKAN
#    define GLFW_INCLUDE_VULKAN
#endif
#include "vkWrappers/wrappers/utils.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace vkw
{
class Instance
{
  public:
    Instance() {}
    Instance(GLFWwindow *window);

    Instance(const Instance &) = delete;
    Instance(Instance &&);

    Instance &operator=(const Instance &) = delete;
    Instance &operator=(Instance &&cp);

    ~Instance();

    void init(GLFWwindow *window);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkInstance &getInstance() { return instance_; }
    const VkInstance &getInstance() const { return instance_; }

    VkSurfaceKHR &getSurface() { return surface_; }
    const VkSurfaceKHR &getSurface() const { return surface_; }

  private:
    GLFWwindow *window_ = nullptr;
    VkInstance instance_{VK_NULL_HANDLE};
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT callback_{nullptr};
    VkDebugReportCallbackEXT reportCallback_{nullptr};

    bool initialized_{false};

    std::vector<VkExtensionProperties> getInstanceExtensionProperties();

    std::vector<VkLayerProperties> getInstanceLayerProperties();

    std::vector<VkPhysicalDevice> listAvailablePhysicalDevices(VkInstance &instance);

    bool checkLayersAvailable(const std::vector<const char *> &layerNames);

    bool checkExtensionsAvailable(const std::vector<const char *> &extensionNames);
};
} // namespace vkw
