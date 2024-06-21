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

namespace vkw
{
/// Helper class to create simple compute programs
class ComputeProgram
{
  public:
    ComputeProgram() = delete;
    ComputeProgram(vkw::Device& device, const char* shaderSource)
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
        for(const auto& bindingInfo : storageBufferBindings_)
        {
            descriptorPool_.bindStorageBuffer(0, bindingInfo.bindingPoint, bindingInfo.bufferInfo);
        }
        for(const auto& bindingInfo : uniformBufferBindings_)
        {
            descriptorPool_.bindUniformBuffer(0, bindingInfo.bindingPoint, bindingInfo.bufferInfo);
        }
        for(const auto& bindingInfo : storageImageBindings_)
        {
            descriptorPool_.bindStorageImage(0, bindingInfo.bindingPoint, bindingInfo.imageInfo);
        }
    }

    template <typename T>
    inline ComputeProgram& bindStorageBuffers(const Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageBufferBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, storageBufferBindingPoint_, 1);
        storageBufferBindings_.emplace_back(
            BufferBinding{storageBufferBindingPoint_, buffer.getFullSizeInfo()});
        storageBufferBindingPoint_++;
        return *this;
    }
    template <typename T, typename... Args>
    inline ComputeProgram& bindStorageBuffers(const Buffer<T>& buffer, Args&&... args)
    {
        bindStorageBuffers(buffer);
        return bindStorageBuffers(std::forward<Args>(args)...);
    }

    template <typename T>
    inline ComputeProgram& bindUniformBuffers(const Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addUniformBufferBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, uniformBufferBindingPoint_, 1);
        uniformBufferBindings_.emplace_back(
            BufferBinding{uniformBufferBindingPoint_, buffer.getFullSizeInfo()});
        uniformBufferBindingPoint_++;
        return *this;
    }
    template <typename T, typename... Args>
    inline ComputeProgram& bindUniformBuffers(const Buffer<T>& buffer, Args&&... args)
    {
        bindUniformBuffers(buffer);
        return bindUniformBuffers(std::forward<Args>(args)...);
    }

    // TODO : handle images

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
    vkw::Device* device_{nullptr};

    uint32_t storageBufferBindingPoint_{0};
    uint32_t uniformBufferBindingPoint_{0};
    uint32_t storageImageBindingPoint_{0};

    vkw::ComputePipeline computePipeline_{};
    vkw::PipelineLayout pipelineLayout_{};
    vkw::DescriptorPool descriptorPool_{};

    struct BufferBinding
    {
        uint32_t bindingPoint;
        VkDescriptorBufferInfo bufferInfo;
    };
    std::vector<BufferBinding> storageBufferBindings_{};
    std::vector<BufferBinding> uniformBufferBindings_{};

    struct ImageBinding
    {
        uint32_t bindingPoint;
        VkDescriptorImageInfo imageInfo;
    };
    std::vector<ImageBinding> storageImageBindings_{};

    uint32_t pushConstantOffset_{0};

    template <QueueFamilyType type>
    friend class CommandBuffer;
};
} // namespace vkw