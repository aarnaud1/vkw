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

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <memory>
#include <fstream>

#include <vulkan/vulkan.h>

#include "vkWrappers/utils.hpp"
#include "vkWrappers/Instance.hpp"
#include "vkWrappers/QueueFamilies.hpp"
#include "vkWrappers/Device.hpp"
#include "vkWrappers/Buffer.hpp"
#include "vkWrappers/PipelineLayout.hpp"
#include "vkWrappers/DescriptorPool.hpp"

namespace vk
{
class ComputePipeline
{
public:
  ComputePipeline(Device &device, const std::string &shaderSource);

  ~ComputePipeline();

  void createPipeline(PipelineLayout &pipelineLayout);

  // One could do some std::tuple stuff to be more efficient
  template <typename T>
  ComputePipeline &addSpec(const T value)
  {
    constexpr size_t size = sizeof(T);
    const char *data = (char *) &value;

    for(size_t i = 0; i < size; i++)
    {
      specData_.push_back(data[i]);
    }

    specSizes_.push_back(size);

    return *this;
  }

  VkPipeline &getHandle() { return pipeline_; }

private:
  Device &device_;
  VkShaderModule shaderModule_;
  VkPipeline pipeline_;

  std::vector<char> specData_;
  std::vector<size_t> specSizes_;

  void createShaderModule(const std::vector<char> &src);

  std::vector<char> readShader(const std::string &filename);
};
} // namespace vk
