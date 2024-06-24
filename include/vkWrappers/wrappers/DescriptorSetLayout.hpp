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

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/QueueFamilies.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class DescriptorSetLayout
{
  public:
    DescriptorSetLayout() = default;

    DescriptorSetLayout(const DescriptorSetLayout &) = default;
    DescriptorSetLayout(DescriptorSetLayout &&) = default;

    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = default;
    DescriptorSetLayout &operator=(DescriptorSetLayout &&) = default;

    ~DescriptorSetLayout() = default;

    void clear()
    {
        bindings_.clear();

        numStorageBufferBindings_ = 0;
        numUniformBufferBindings_ = 0;
        numStorageImageBindings_ = 0;
    }

    DescriptorSetLayout &addStorageBufferBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
    DescriptorSetLayout &addUniformBufferBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
    DescriptorSetLayout &addStorageImageBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
    DescriptorSetLayout &addSamplerImageBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);

    std::vector<VkDescriptorSetLayoutBinding> &getBindings() { return bindings_; }

    uint32_t getNumStorageBufferBindings() const { return numStorageBufferBindings_; }
    uint32_t getNumUniformBufferBindings() const { return numUniformBufferBindings_; }
    uint32_t getNumStorageImageBindings() const { return numStorageImageBindings_; }
    uint32_t getNumSamplerImageBindings() const { return numSamplerImageBindings_; }

  private:
    std::vector<VkDescriptorSetLayoutBinding> bindings_{};
    uint32_t numStorageBufferBindings_ = 0;
    uint32_t numUniformBufferBindings_ = 0;
    uint32_t numStorageImageBindings_ = 0;
    uint32_t numSamplerImageBindings_ = 0;
};
} // namespace vkw
