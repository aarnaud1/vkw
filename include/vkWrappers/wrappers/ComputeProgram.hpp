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
#include "vkWrappers/wrappers/ComputePipeline.hpp"
#include "vkWrappers/wrappers/ImageView.hpp"

#include <type_traits>

namespace vkw
{
struct EmptyComputeParams
{};

/// Helper class to create simple compute programs
template <typename Params = EmptyComputeParams>
class ComputeProgram
{
  public:
    ComputeProgram() {}
    ComputeProgram(Device& device, const char* shaderSource) { this->init(device, shaderSource); }

    ComputeProgram(const ComputeProgram&) = delete;
    ComputeProgram(ComputeProgram&& cp) { *this = std::move(cp); }

    ComputeProgram& operator=(const ComputeProgram&) = delete;
    ComputeProgram& operator=(ComputeProgram&& cp)
    {
        this->clear();
        std::swap(device_, cp.device_);

        computePipeline_ = std::move(cp.computePipeline_);
        pipelineLayout_ = std::move(cp.pipelineLayout_);
        return *this;
    }

    ~ComputeProgram() { this->clear(); }

    bool isInitialized() const { return initialized_; }

    void init(Device& device, const char* shaderSource)
    {
        if(!initialized_)
        {
            device_ = &device;
            computePipeline_.init(device, shaderSource);
            pipelineLayout_.init(device, 1);

            initialized_ = true;
        }
    }

    void clear()
    {
        if(initialized_)
        {
            computePipeline_ = {};
            pipelineLayout_ = {};
            descriptorPool_ = {};

            initialized_ = false;
        }
        device_ = nullptr;
    }

    void create()
    {
        if constexpr(std::is_empty<Params>::value == false)
        {
            pushConstantOffset_
                = pipelineLayout_.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(Params));
        }

        pipelineLayout_.create();
        computePipeline_.createPipeline(pipelineLayout_);
        descriptorPool_.init(*device_, pipelineLayout_);
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

    auto& computePipeline() { return computePipeline_; }
    const auto& computePipeline() const { return computePipeline_; }

    template <typename T>
    inline ComputeProgram& bindStorageBuffer(const uint32_t bindingPoint, const Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageBufferBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, bindingPoint, 1);
        storageBufferBindings_.emplace_back(BufferBinding{bindingPoint, buffer.getFullSizeInfo()});
        return *this;
    }

    template <typename T>
    inline ComputeProgram& bindUniformBuffer(const uint32_t bindingPoint, const Buffer<T>& buffer)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addUniformBufferBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, bindingPoint, 1);
        uniformBufferBindings_.emplace_back(BufferBinding{bindingPoint, buffer.getFullSizeInfo()});
        return *this;
    }

    inline ComputeProgram& bindStorageImage(const uint32_t bindingPoint, const ImageView& image)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageImageBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, bindingPoint, 1);
        storageImageBindings_.emplace_back(
            ImageBinding{bindingPoint, {nullptr, image.getHandle(), VK_IMAGE_LAYOUT_GENERAL}});
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

  private:
    Device* device_{nullptr};
    bool initialized_{false};

    ComputePipeline computePipeline_{};
    PipelineLayout pipelineLayout_{};
    DescriptorPool descriptorPool_{};

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

    friend class CommandBuffer;
};
} // namespace vkw