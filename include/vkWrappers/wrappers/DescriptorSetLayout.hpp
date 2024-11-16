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
    DescriptorSetLayout() {}

    DescriptorSetLayout(Device &device);

    DescriptorSetLayout(const DescriptorSetLayout &) = delete;
    DescriptorSetLayout(DescriptorSetLayout &&cp) { *this = std::move(cp); }

    DescriptorSetLayout &operator=(const DescriptorSetLayout &) = default;
    DescriptorSetLayout &operator=(DescriptorSetLayout &&);

    ~DescriptorSetLayout() = default;

    bool isInitialized() const { return initialized_; }

    bool init(Device &device);

    void clear();

    DescriptorSetLayout &addStorageBufferBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
    DescriptorSetLayout &addUniformBufferBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
    DescriptorSetLayout &addStorageImageBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);
    DescriptorSetLayout &addSamplerImageBinding(
        VkShaderStageFlags flags, uint32_t bindingPoint, uint32_t bindingCount);

    void create();

    std::vector<VkDescriptorSetLayoutBinding> &getBindings() { return bindings_; }

    uint32_t storageBufferBindingCount() const { return storageBufferBindingCount_; }
    uint32_t uniformBufferBindingCount() const { return uniformBufferBindingCount_; }
    uint32_t storageImageBindingCount() const { return storageImageBindingCount_; }
    uint32_t combinedImageSamplerBindingCount() const { return combinedImageSamplerBindingCount_; }

    VkDescriptorSetLayout getHandle() const { return descriptorSetLayout_; }

  private:
    Device *device_{nullptr};
    VkDescriptorSetLayout descriptorSetLayout_{VK_NULL_HANDLE};

    std::vector<VkDescriptorSetLayoutBinding> bindings_{};
    uint32_t storageBufferBindingCount_ = 0;
    uint32_t uniformBufferBindingCount_ = 0;
    uint32_t storageImageBindingCount_ = 0;
    uint32_t combinedImageSamplerBindingCount_ = 0;

    bool initialized_{false};
};
} // namespace vkw
