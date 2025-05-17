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

#include "vkw/detail/Common.hpp"
#include "vkw/detail/utils.hpp"

#include <cstdio>
#include <cstring>
#include <vector>

namespace vkw
{
// Call it before any Vulkan call in case you don't build an instance first
VkResult initializeVulkan();

class Instance
{
  public:
    Instance() {}
    Instance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

    Instance(const Instance&) = delete;
    Instance(Instance&& rhs);

    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&& rhs);

    ~Instance();

    bool init(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkInstance& getHandle() { return instance_; }
    const VkInstance& getHandle() const { return instance_; }

  private:
    VkInstance instance_{VK_NULL_HANDLE};

    bool initialized_{false};

    std::vector<VkExtensionProperties> getInstanceExtensionProperties();

    std::vector<VkLayerProperties> getInstanceLayerProperties();

    bool checkLayersAvailable(const std::vector<const char*>& layerNames);
};
} // namespace vkw
