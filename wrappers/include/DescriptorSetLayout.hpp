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
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "utils.hpp"
#include "Instance.hpp"
#include "QueueFamilies.hpp"
#include "Device.hpp"
#include "Buffer.hpp"

namespace vk
{
class DescriptorSetLayout
{
public:
  DescriptorSetLayout();

  ~DescriptorSetLayout();

  void createResources();

  DescriptorSetLayout &
  addStorageBufferBinding(VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
  DescriptorSetLayout &
  addUniformBufferBinding(VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
  DescriptorSetLayout &
  addStorageImageBinding(VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);

  std::vector<VkDescriptorSetLayoutBinding> &getBindings() { return bindings_; }

  uint32_t getNumStorageBufferBindings() const { return numStorageBufferBindings_; }

  uint32_t getNumUniformBufferBindings() const { return numUniformBufferBindings_; }

  uint32_t getNumStorageImageBindings() const { return numStorageImageBindings_; }

private:
  std::vector<VkDescriptorSetLayoutBinding> bindings_;
  uint32_t numStorageBufferBindings_ = 0;
  uint32_t numUniformBufferBindings_ = 0;
  uint32_t numStorageImageBindings_ = 0;
};
} // namespace vk
