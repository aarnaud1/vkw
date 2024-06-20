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

#include "vkWrappers/wrappers/ComputePipeline.hpp"

namespace vk
{
/// Helper class to create simple compute programs
class ComputeProgram
{
  public:
    ComputeProgram() = delete;
    ComputeProgram(vk::Device& device, const char* shaderSource)
        : device_{&device}, computePipeline_{device, shaderSource}, pipelineLayout_{device, 1}
    {}

    ComputeProgram(const ComputeProgram&) = delete;
    ComputeProgram(ComputeProgram&&) = delete;

    ComputeProgram& operator=(const ComputeProgram&) = delete;
    ComputeProgram& operator&&(ComputeProgram&&) = delete;

    inline void create()
    {
        pipelineLayout_.create();
        computePipeline_.createPipeline(pipelineLayout_);
        descriptorPool_.init(*device_, pipelineLayout_, VK_SHADER_STAGE_COMPUTE_BIT);
        for(const auto& bindingInfo : bufferBindings_)
        {
            descriptorPool_.bindStorageBuffer(0, bindingInfo.bindingPoint, bindingInfo.bufferInfo);
        }
    }

    template <typename T>
    inline ComputeProgram& bindBuffer(const vk::Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageBufferBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, bindingPoint_, 1);
        bufferBindings_.emplace_back(BufferBinding{bindingPoint_, buffer.getFullSizeInfo()});
        bindingPoint_++;
        return *this;
    }

    template <typename T>
    inline ComputeProgram& spec(const T val)
    {
        computePipeline_.addSpec<T>(val);
        return *this;
    }
    template <typename T, typename... Args>
    inline ComputeProgram& spec(const T val, Args&&... args)
    {
        computePipeline_.addSpec<T>(val);
        return spec(std::forward<Args>(args)...);
    }

    inline ComputeProgram& pushConstantsRange(const size_t range)
    {
        pushConstantOffset_
            = pipelineLayout_.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, range);
        return *this;
    }

  private:
    vk::Device* device_{nullptr};

    uint32_t bindingPoint_{0};

    vk::ComputePipeline computePipeline_{};
    vk::PipelineLayout pipelineLayout_{};
    vk::DescriptorPool descriptorPool_{};

    struct BufferBinding
    {
        uint32_t bindingPoint;
        VkDescriptorBufferInfo bufferInfo;
    };
    std::vector<BufferBinding> bufferBindings_{};
    uint32_t pushConstantOffset_{0};

    template <QueueFamilyType type>
    friend class CommandBuffer;
};

} // namespace vk