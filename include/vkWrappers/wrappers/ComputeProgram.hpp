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
/// Helper class to create simple compute programs
template <typename Params>
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
        std::swap(storageBufferBindingPoint_, cp.storageBufferBindingPoint_);
        std::swap(uniformBufferBindingPoint_, cp.uniformBufferBindingPoint_);
        std::swap(storageImageBindingPoint_, cp.storageImageBindingPoint_);

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
            storageBufferBindingPoint_ = 0;
            uniformBufferBindingPoint_ = 0;
            storageImageBindingPoint_ = 0;

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

    inline ComputeProgram& bindStorageImages(const ImageView& image)
    {
        pipelineLayout_.getDescriptorSetlayoutInfo(0).addStorageImageBinding(
            VK_SHADER_STAGE_COMPUTE_BIT, storageImageBindingPoint_, 1);
        storageImageBindings_.emplace_back(ImageBinding{
            storageImageBindingPoint_, {nullptr, image.getHandle(), VK_IMAGE_LAYOUT_GENERAL}});
        storageImageBindingPoint_++;
        return *this;
    }
    template <typename... Args>
    inline ComputeProgram& bindStorageImages(const ImageView& image, Args&&... args)
    {
        bindStorageImages(image);
        return bindStorageImages(std::forward<Args>(args)...);
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

    uint32_t storageBufferBindingPoint_{0};
    uint32_t uniformBufferBindingPoint_{0};
    uint32_t storageImageBindingPoint_{0};

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