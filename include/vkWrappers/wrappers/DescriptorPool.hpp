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
#include "vkWrappers/wrappers/DescriptorSetLayout.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/PipelineLayout.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class DescriptorPool
{
  public:
    DescriptorPool() {}
    DescriptorPool(Device &device, PipelineLayout &pipelineLayout, VkShaderStageFlags flags);

    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool(DescriptorPool &&cp) { *this = std::move(cp); }

    DescriptorPool &operator=(const DescriptorPool &) = delete;
    DescriptorPool &operator=(DescriptorPool &&cp)
    {
        this->clear();

        std::swap(cp.device_, device_);
        std::swap(cp.descriptorSets_, descriptorSets_);
        std::swap(cp.descriptorPool_, descriptorPool_);
        std::swap(cp.initialized_, initialized_);

        return *this;
    }

    ~DescriptorPool();

    void init(Device &device, PipelineLayout &pipelineLayout, VkShaderStageFlags flags);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkDescriptorPool &getHandle() { return descriptorPool_; }
    const VkDescriptorPool &getHandle() const { return descriptorPool_; }

    std::vector<VkDescriptorSet> &getDescriptors() { return descriptorSets_; }

    DescriptorPool &bindStorageBuffer(
        uint32_t setId,
        uint32_t bindingId,
        VkDescriptorBufferInfo bufferInfo,
        uint32_t offset = 0,
        uint32_t count = 1);

    DescriptorPool &bindStorageImage(
        uint32_t setId,
        uint32_t bindingId,
        VkDescriptorImageInfo imageInfo,
        uint32_t offset = 0,
        uint32_t count = 1);

    DescriptorPool &bindUniformBuffer(
        uint32_t setId,
        uint32_t bindingId,
        VkDescriptorBufferInfo bufferInfo,
        uint32_t offset = 0,
        uint32_t count = 1);

    DescriptorPool &bindSamplerImage(
        uint32_t setId,
        uint32_t bindingId,
        VkDescriptorImageInfo imageInfo,
        uint32_t offset = 0,
        uint32_t count = 1);

  private:
    Device *device_{nullptr};
    std::vector<VkDescriptorSet> descriptorSets_{};
    VkDescriptorPool descriptorPool_{VK_NULL_HANDLE};

    bool initialized_{false};

    void allocateDescriptorSets(PipelineLayout &pipelineLayout);
};
} // namespace vkw
