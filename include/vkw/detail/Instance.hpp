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
    Instance(
        const VkApplicationInfo& info, const std::vector<const char*>& layers,
        const std::vector<const char*>& extensions);
    Instance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

    Instance(const Instance&) = delete;
    Instance(Instance&& rhs);

    Instance& operator=(const Instance&) = delete;
    Instance& operator=(Instance&& rhs);

    ~Instance();

    bool init(
        const VkApplicationInfo info, const std::vector<const char*>& layers,
        const std::vector<const char*>& extensions);
    bool init(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

    void clear();

    bool initialized() const { return initialized_; }

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
