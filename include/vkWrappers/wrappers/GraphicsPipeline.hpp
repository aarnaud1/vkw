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
#include "vkWrappers/wrappers/DescriptorPool.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/PipelineLayout.hpp"
#include "vkWrappers/wrappers/QueueFamilies.hpp"
#include "vkWrappers/wrappers/RenderPass.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <array>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk
{
class GraphicsPipeline
{
  public:
    GraphicsPipeline() {}
    GraphicsPipeline(Device &device, const bool useDepth = true);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&);

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&cp);

    ~GraphicsPipeline();

    void init(Device &device, const bool useDepth = true);

    void clear();

    bool isInitialized() const { return initialized_; }

    GraphicsPipeline &addShaderStage(
        const VkShaderStageFlagBits stage, const std::string &shaderSource);

    GraphicsPipeline &addVertexBinding(
        const uint32_t binding,
        const uint32_t stride,
        const VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

    GraphicsPipeline &addVertexAttribute(
        const uint32_t location,
        const uint32_t binding,
        const VkFormat format,
        const uint32_t offset);

    GraphicsPipeline &setPrimitiveType(
        const VkPrimitiveTopology primitive, const VkBool32 primitiveEnableRestart = false);

    GraphicsPipeline &setViewport(const VkViewport &viewport)
    {
        viewport_ = viewport;
        return *this;
    }
    GraphicsPipeline &setViewport(
        const float x,
        const float y,
        const float w,
        const float h,
        const float minDepth = 0.0f,
        const float maxDepth = 0.0f)
    {
        viewport_ = VkViewport{x, y, w, h, minDepth, maxDepth};
        return *this;
    }

    GraphicsPipeline &setScissors(
        const int32_t offX, const int32_t offY, const uint32_t w, const uint32_t h)
    {
        scissor_ = VkRect2D{{offX, offY}, {w, h}};
        return *this;
    }

    GraphicsPipeline &enableBlending(const bool useBlending)
    {
        useBlending_ = useBlending;
        return *this;
    }

    template <typename T>
    GraphicsPipeline &addSpec(const VkShaderStageFlagBits stage, const T value)
    {
        static constexpr size_t size = sizeof(T);
        const char *data = (char *) &value;

        const int id = getStageIndex(stage);
        if(id > 0)
        {
            auto &info = moduleInfo_[id];
            for(size_t i = 0; i < size; i++)
            {
                info.specData.push_back(data[i]);
            }
            info.specSizes.push_back(size);
        }

        return *this;
    }

    void createPipeline(
        RenderPass &renderPass, PipelineLayout &pipelineLayout, const uint32_t subPass = 0);

    VkPipeline &getHandle() { return pipeline_; }
    const VkPipeline &getHandle() const { return pipeline_; }

  private:
    static constexpr size_t maxStageCount = 5;

    Device *device_{nullptr};
    VkPipeline pipeline_{VK_NULL_HANDLE};
    std::vector<VkVertexInputBindingDescription> bindingDescriptions_{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions_{};
    VkViewport viewport_{};
    VkRect2D scissor_{0, 0, 0, 0};
    bool useDepth_{true};
    bool useBlending_{false};

    VkPrimitiveTopology topology_{VK_PRIMITIVE_TOPOLOGY_POINT_LIST};
    VkBool32 primitiveEnableRestart_ = VK_FALSE;

    struct ShaderModuleInfo
    {
        std::string shaderSource{};
        VkShaderModule shaderModule{VK_NULL_HANDLE};
        std::vector<char> specData{};
        std::vector<size_t> specSizes{};
    };
    std::array<ShaderModuleInfo, maxStageCount> moduleInfo_{};

    bool initialized_{false};

    static inline int32_t getStageIndex(const VkShaderStageFlagBits stage)
    {
        if(stage == VK_SHADER_STAGE_VERTEX_BIT)
        {
            return 0;
        }
        else if(stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
        {
            return 1;
        }
        else if(stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
        {
            return 2;
        }
        else if(stage == VK_SHADER_STAGE_GEOMETRY_BIT)
        {
            return 3;
        }
        else if(stage == VK_SHADER_STAGE_FRAGMENT_BIT)
        {
            return 4;
        }

        return -1;
    }
};
} // namespace vk