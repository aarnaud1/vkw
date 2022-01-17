/*
 * Copyright (C) 2022 Adrien ARNAUD
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

#include <vector>
#include <cstdio>
#include <cstring>

#ifndef GLFW_INCLUDE_VULKAN
#  define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include "utils.hpp"

namespace vk
{
class Instance
{
public:
  Instance(GLFWwindow *window);

  ~Instance();

  inline VkInstance getInstance() { return instance_; }

  inline VkSurfaceKHR getSurface() { return surface_; }

private:
  GLFWwindow *window_ = nullptr;
  VkInstance instance_;
  VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT callback_;

  std::vector<VkExtensionProperties> getInstanceExtensionProperties();

  std::vector<VkLayerProperties> getInstanceLayerProperties();

  std::vector<VkPhysicalDevice>
  listAvailablePhysicalDevices(VkInstance &instance);

  bool checkLayersAvailable(const std::vector<const char *> &layerNames);

  bool
  checkExtensionsAvailable(const std::vector<const char *> &extensionNames);
};
} // namespace vk
