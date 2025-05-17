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

#include "vkw/vkw.hpp"

namespace vkw
{
struct EmptyParams
{};

template <typename PushConstants, DescriptorType... descriptorTypes>
class ComputeProgram
{
  public:
    using constant_type = PushConstants;

    ComputeProgram() = delete;
    ComputeProgram(
        const Device& device,
        const std::string& shaderSource,
        const uint32_t descriptorSetCount = 1)
        : device_{device}, descriptorSetCount_{descriptorSetCount}
    {
        descriptorSetLayout_.init(device_);
        addDescriptors(descriptorTypes, 0);
        descriptorSetLayout_.create();

        pipelineLayout_.init(device_, descriptorSetLayout_);
        pipelineLayout_.reservePushConstants<PushConstants>(ShaderStage::Compute).create();

        computePipeline_.init(device, shaderSource);
    }

    template <typename... Args>
    ComputeProgram& spec(Args...&& args)
    {
        computePipeline_.addSpec(std::forward<Args>(args)...);
        return *this;
    }

    ComputeProgram& build()
    {
        VKW_CHECK_BOOL_RETURN_FALSE(computePipeline_.createPipeline(pipelineLayout_));

        std::vector<VkDescriptorPoolSize> poolSizes = {};
        for(uint32_t i = 0; i < vkw::descriptorTypeCount; ++i)
        {
            const auto count = descriptorCounts_[i];
            if(count > 0)
            {
                VkDescriptorPoolSize poolSize = {};
                poolSize.type = vkw::getVkDescriptorType(static_cast<DescriptorType>(i));
                poolSize.descriptorCount = count * descriptorSetCount_;
                poolSizes.push_back(poolSize);
            }
        }
        VKW_CHECK_BOOL_RETURN_FALSE(descriptorPool_.init(device_, descriptorSetCount_, poolSizes));
        descriptorSets_
            = descriptorPool_.allocateDescriptorSets(descriptorSetLayout_, descriptorSetCount_);

        return *this;
    }

    auto& descriptorSet(const size_t i) { return descriptorSets_.at(i); }
    const auto& descriptorSet(const size_t i) const { return descriptorSets_.at(i); }

    const auto& device() const { return device_; }
    const auto& pipeline() const { return computePipeline_ };
    const auto& pipelineLayout pipelineLayout() const { return pipelineLayout_; }

  private:
    friend class CommandBuffer;

    Device& device_;
    const uint32_t descriptorSetCount_;

    ComputePipeline computePipeline_{};

    DescriptorSetLayout descriptorSetLayout_{};
    PipelineLayout pipelineLayout_{};

    DescriptorPool descriptorPool_{};
    std::vector<DescriptorSet> descriptorSets_{};

    uint32_t descriptorCounts_[vkw::descriptorTypeCount]{};

    // Initialize descriptor set layout
    template <DescriptorType>
    void addDescriptor(const uint32_t binding, const DescriptorType descriptorType)
    {
        descriptorSetLayout_.addBinding<descriptorType>(VK_SHADER_STAGE_COMPUTE_BIT, binding);
        descriptorCounts_[static_cast<uint32_t>(descriptorType)]++;
    }
    template <DescriptorType... descriptorTypes>
    void addDescriptors(
        const uint32_t binding, DescriptorType descriptorType, DescriptorTypes...&& descriptorTypes)
    {
        addDescriptor(binding, descriptorType);
        addDescriptors(binding + 1, std::forward<DescriptorTypes>(descriptorTypes)...);
    }
};
} // namespace vkw