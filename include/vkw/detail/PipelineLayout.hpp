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
#include "vkw/detail/DescriptorSetLayout.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Instance.hpp"

#include <cstdlib>
#include <vector>

namespace vkw
{
enum class ShaderStage : uint32_t
{
    Vertex = 0,
    TesselationControl = 1,
    TesselationEvaluation = 2,
    Geometry = 3,
    Fragment = 4,
    Compute = 5,
    Task = 6,
    Mesh = 7,
    Raygen = 8,
    AnyHit = 9,
    ClosestHit = 10,
    Miss = 11,
    Intersection = 12,
    Callable = 13
};

typedef uint32_t ShaderStageFlags;
static constexpr size_t shaderStageCount = 14;

class PipelineLayout
{
  public:
    PipelineLayout() = default;

    PipelineLayout(Device& device)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device), "Initializing ipeline layout");
    }

    PipelineLayout(Device& device, DescriptorSetLayout& descriptorSetLayout)
    {
        VKW_CHECK_BOOL_FAIL(
            this->init(device, descriptorSetLayout), "Initializing pipeline layout");
    }

    template <typename... Args>
    PipelineLayout(Device& device, DescriptorSetLayout& descriptorSetLayout, Args&&... args)
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

    bool init(Device& device);

    bool init(Device& device, DescriptorSetLayout& descriptorSetLayout)
    {
        descriptorSetLayouts_.push_back(descriptorSetLayout.getHandle());
        return init(device);
    }

    template <typename... Args>
    bool init(Device& device, DescriptorSetLayout& descriptorSetLayout, Args&&... args)
    {
        descriptorSetLayouts_.push_back(descriptorSetLayout.getHandle());
        return init(device, std::forward<Args>(args)...);
    }

    void clear();

    void create();

    bool initialized() const { return initialized_; }

    VkPipelineLayout getHandle() const { return pipelineLayout_; }

    template <typename T>
    PipelineLayout& reservePushConstants(const ShaderStage stage)
    {
        VKW_CHECK_BOOL_FAIL(
            ranges_[static_cast<uint32_t>(stage)].size == 0,
            "Range already allocated for this shader stage");

        const uint32_t size = utils::alignedSize(static_cast<uint32_t>(sizeof(T)), uint32_t(4));
        ranges_[static_cast<uint32_t>(stage)].offset = offset_;
        ranges_[static_cast<uint32_t>(stage)].size = size;
        ranges_[static_cast<uint32_t>(stage)].stageFlags = getVkShaderStage(stage);
        offset_ += size;

        return *this;
    }

    template <typename T, typename... Args>
    PipelineLayout& reservePushConstants(const ShaderStage stage, Args&&... args)
    {
        reservePushConstants<T>(stage);
        return reservePushConstants<T>(std::forward<Args>(args)...);
    }

    size_t descriptorSetCount() const { return descriptorSetLayouts_.size(); }

  private:
    friend class CommandBuffer;

    Device* device_{nullptr};

    uint32_t offset_{0};
    VkPushConstantRange ranges_[shaderStageCount]{};

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts_{};
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};

    bool initialized_{false};

    static inline VkShaderStageFlagBits getVkShaderStage(const ShaderStage stage)
    {
        switch(stage)
        {
            case ShaderStage::Vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderStage::TesselationControl:
                return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            case ShaderStage::TesselationEvaluation:
                return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            case ShaderStage::Geometry:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            case ShaderStage::Fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderStage::Compute:
                return VK_SHADER_STAGE_COMPUTE_BIT;
            case ShaderStage::Task:
                return VK_SHADER_STAGE_TASK_BIT_EXT;
            case ShaderStage::Mesh:
                return VK_SHADER_STAGE_MESH_BIT_EXT;
            case ShaderStage::Raygen:
                return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            case ShaderStage::AnyHit:
                return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            case ShaderStage::ClosestHit:
                return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            case ShaderStage::Miss:
                return VK_SHADER_STAGE_MISS_BIT_KHR;
            case ShaderStage::Intersection:
                return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
            case ShaderStage::Callable:
                return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
            default:
                return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }
};
} // namespace vkw