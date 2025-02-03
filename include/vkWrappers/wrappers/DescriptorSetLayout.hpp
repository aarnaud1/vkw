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
enum DescriptorType
{
    Sampler = 0,
    CombinedImageSampler = 1,
    SampledImage = 2,
    StorageImage = 3,
    UniformTexelBuffer = 4,
    StorageTexelBuffer = 5,
    UniformBuffer = 6,
    StorageBuffer = 7,
    UniformBufferDynamic = 8,
    StorageBufferDynamic = 9,
    InputAttachment = 10
};
static constexpr size_t descriptorTypeCount = 11;

static inline VkDescriptorType getVkDescriptorType(const DescriptorType type)
{
    static const VkDescriptorType descriptorTypes[descriptorTypeCount]
        = {VK_DESCRIPTOR_TYPE_SAMPLER,
           VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
           VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
           VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
           VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
           VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
           VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
           VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
           VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
           VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT};

    const auto i = static_cast<uint32_t>(type);
    return descriptorTypes[i];
}

class DescriptorSetLayout
{
  public:
    DescriptorSetLayout() {}

    DescriptorSetLayout(Device& device);

    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& cp) { *this = std::move(cp); }

    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = default;
    DescriptorSetLayout& operator=(DescriptorSetLayout&&);

    ~DescriptorSetLayout() = default;

    bool isInitialized() const { return initialized_; }

    bool init(Device& device);

    void clear();

    template <DescriptorType type>
    inline DescriptorSetLayout& addBinding(
        const VkShaderStageFlags flags, const uint32_t binding, const uint32_t count = 1)
    {
        VkDescriptorSetLayoutBinding bindingInfo = {};
        bindingInfo.binding = binding;
        bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindingInfo.descriptorCount = count;
        bindingInfo.stageFlags = flags;
        bindingInfo.pImmutableSamplers = nullptr;

        bindings_.push_back(bindingInfo);
        bindingCounts_[type]++;
        return *this;
    }

    void create();

    std::vector<VkDescriptorSetLayoutBinding>& getBindings() { return bindings_; }

    template <DescriptorType type>
    uint32_t DescriptorCount() const
    {
        return bindingCounts_[type];
    }

    uint32_t totalBindingCount() const
    {
        uint32_t sum = 0;
        for(size_t i = 0; i < descriptorTypeCount; ++i)
        {
            sum += bindingCounts_[i];
        }
        return sum;
    }

    VkDescriptorSetLayout getHandle() const { return descriptorSetLayout_; }

    const auto& bindingList() const { return bindings_; }

  private:
    Device* device_{nullptr};
    VkDescriptorSetLayout descriptorSetLayout_{VK_NULL_HANDLE};

    uint32_t bindingCounts_[descriptorTypeCount]{};
    std::vector<VkDescriptorSetLayoutBinding> bindings_{};

    bool initialized_{false};
};
} // namespace vkw
