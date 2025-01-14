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

#pragma once

#include "vkWrappers/wrappers/extensions/InstanceExtensions.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class Instance
{
  public:
    Instance() {};
    Instance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

    Instance(const Instance&) = delete;
    Instance(Instance&&);

    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&& cp);

    ~Instance();

    bool init(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkInstance& getHandle() { return instance_; }
    const VkInstance& getHandle() const { return instance_; }

    void setSurface(VkSurfaceKHR&& surface)
    {
        if(surface_ != VK_NULL_HANDLE)
        {
            throw std::runtime_error("Instance already has a surface");
        }
        std::swap(surface_, surface);
    }

    VkSurfaceKHR& getSurface() { return surface_; }
    const VkSurfaceKHR& getSurface() const { return surface_; }

  private:
    VkInstance instance_{VK_NULL_HANDLE};
    VkSurfaceKHR surface_{VK_NULL_HANDLE};

    bool initialized_{false};

    std::vector<VkExtensionProperties> getInstanceExtensionProperties();

    std::vector<VkLayerProperties> getInstanceLayerProperties();

    bool checkLayersAvailable(const std::vector<const char*>& layerNames);
};
} // namespace vkw
