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

#include "vkWrappers/wrappers/DescriptorSet.hpp"
#include "vkWrappers/wrappers/DescriptorSetLayout.hpp"
#include "vkWrappers/wrappers/Device.hpp"

#include <vector>

namespace vkw
{
class DescriptorPool
{
  public:
    DescriptorPool() {}
    DescriptorPool(Device& device, const uint32_t maxSetCount, const uint32_t maxPoolSize);

    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool(DescriptorPool&& cp) { *this = std::move(cp); }

    DescriptorPool& operator=(const DescriptorPool&) = delete;
    DescriptorPool& operator=(DescriptorPool&& cp);

    ~DescriptorPool() { this->clear(); }

    bool init(Device& device, const uint32_t maxSetCount, const uint32_t minPoolSize);

    void clear();

    bool isInitialized() const { return initialized_; }

    DescriptorSet allocateDescriptorSet(const DescriptorSetLayout& layout);

    std::vector<DescriptorSet> allocateDescriptorSets(
        const DescriptorSetLayout& layout, const uint32_t count);

  private:
    Device* device_{nullptr};
    std::vector<VkDescriptorSet> descriptorSets_{};
    VkDescriptorPool descriptorPool_{VK_NULL_HANDLE};

    uint32_t maxSetCount_{0};
    uint32_t maxPoolSize_{0};

    bool initialized_{false};
};
} // namespace vkw