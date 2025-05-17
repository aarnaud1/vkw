/*
 * Copyright (c) 2025 Adrien ARNAUD
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

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Instance.hpp"
#include "vkw/detail/utils.hpp"

#include <string>
#include <vector>

namespace vkw
{
enum class DescriptorType : uint32_t
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
    InputAttachment = 10,
    AccelerationStructure = 11
};
static constexpr size_t descriptorTypeCount = 12;

static inline constexpr VkDescriptorType getVkDescriptorType(const DescriptorType type)
{
    constexpr VkDescriptorType descriptorTypes[descriptorTypeCount]
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
           VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
           VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR};
    return descriptorTypes[static_cast<uint32_t>(type)];
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

    ~DescriptorSetLayout()
    {
        VKW_DELETE_VK(DescriptorSetLayout, descriptorSetLayout_);
        device_ = nullptr;
        memset(bindingCounts_, 0, descriptorTypeCount * sizeof(uint32_t));
        bindings_.clear();

        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    bool init(Device& device);

    void clear();

    template <DescriptorType type>
    inline DescriptorSetLayout& addBinding(
        const VkShaderStageFlags flags, const uint32_t binding, const uint32_t count = 1)
    {
        VkDescriptorSetLayoutBinding bindingInfo = {};
        bindingInfo.binding = binding;
        bindingInfo.descriptorType = getVkDescriptorType(type);
        bindingInfo.descriptorCount = count;
        bindingInfo.stageFlags = flags;
        bindingInfo.pImmutableSamplers = nullptr;

        bindings_.push_back(bindingInfo);
        bindingCounts_[static_cast<uint32_t>(type)]++;
        return *this;
    }

    void create();

    std::vector<VkDescriptorSetLayoutBinding>& getBindings() { return bindings_; }

    template <DescriptorType type>
    uint32_t DescriptorCount() const
    {
        return bindingCounts_[static_cast<uint32_t>(type)];
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
