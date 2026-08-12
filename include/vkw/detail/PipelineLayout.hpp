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

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/DescriptorSetLayout.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Instance.hpp"

#include <cstdlib>
#include <vector>

namespace vkw
{
class PipelineLayout
{
  public:
    PipelineLayout() = default;

    PipelineLayout(const Device& device)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device), "Initializing ipeline layout");
    }

    PipelineLayout(const Device& device, DescriptorSetLayout& descriptorSetLayout)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, descriptorSetLayout), "Initializing pipeline layout");
    }

    PipelineLayout(
        const Device& device, const std::vector<std::reference_wrapper<DescriptorSetLayout>>& layouts)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, layouts), "Initialiwing pipeline layout");
    }

    template <typename... Args>
    PipelineLayout(const Device& device, DescriptorSetLayout& descriptorSetLayout, Args&&... args)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(device, descriptorSetLayout, std::forward<Args>(args)...),
            "Initializing pipeline layout");
    }

    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout(PipelineLayout&& cp);

    PipelineLayout& operator=(const PipelineLayout&) = delete;
    PipelineLayout& operator=(PipelineLayout&& cp);

    ~PipelineLayout() { this->clear(); }

    bool init(const Device& device);

    bool init(const Device& device, DescriptorSetLayout& descriptorSetLayout)
    {
        descriptorSetLayouts_.push_back(descriptorSetLayout.getHandle());
        return init(device);
    }

    bool init(const Device& device, const std::vector<std::reference_wrapper<DescriptorSetLayout>>& layouts)
    {
        for(auto& layout : layouts)
        {
            descriptorSetLayouts_.emplace_back(layout.get().getHandle());
        }
        return init(device);
    }

    template <typename... Args>
    bool init(const Device& device, DescriptorSetLayout& descriptorSetLayout, Args&&... args)
    {
        descriptorSetLayouts_.push_back(descriptorSetLayout.getHandle());
        return init(device, std::forward<Args>(args)...);
    }

    void clear();

    bool create();

    bool initialized() const { return initialized_; }

    VkPipelineLayout getHandle() const { return pipelineLayout_; }

    template <typename T>
    PipelineLayout& reservePushConstants(const VkShaderStageFlags flags)
    {
        ranges_.emplace_back();

        const uint32_t size = utils::alignedSize(static_cast<uint32_t>(sizeof(T)), uint32_t(4));
        ranges_.back().offset = offset_;
        ranges_.back().size = size;
        ranges_.back().stageFlags = flags;
        offset_ += size;

        return *this;
    }

    template <typename T, typename... Args>
    PipelineLayout& reservePushConstants(const VkShaderStageFlags stages, Args&&... args)
    {
        reservePushConstants<T>(stages);
        return reservePushConstants<T>(std::forward<Args>(args)...);
    }

    size_t descriptorSetCount() const { return descriptorSetLayouts_.size(); }

  private:
    friend class CommandBuffer;

    const Device* device_{nullptr};

    uint32_t offset_{0};
    std::vector<VkPushConstantRange> ranges_{};

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts_{};
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
