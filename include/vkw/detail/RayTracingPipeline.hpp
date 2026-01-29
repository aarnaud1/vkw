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

#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/PipelineLayout.hpp"

#include <array>
#include <vector>

namespace vkw
{
class RayTracingPipeline
{
  public:
    constexpr RayTracingPipeline() {}
    explicit RayTracingPipeline(const Device& device);

    RayTracingPipeline(const RayTracingPipeline&) = delete;
    RayTracingPipeline(RayTracingPipeline&&);

    RayTracingPipeline& operator=(const RayTracingPipeline&) = delete;
    RayTracingPipeline& operator=(RayTracingPipeline&& cp);

    ~RayTracingPipeline();

    bool init(const Device& device);

    void clear();

    bool initialized() const { return initialized_; }

    /// @todo Add the possibility to use a VkShaderModule to initialize a shader stage.
    bool addShaderStage(
        const VkShaderStageFlags stage, const std::string& shaderSource, const char* pName = nullptr);
    bool addShaderStage(
        const VkShaderStageFlags stage, const char* srcData, const size_t byteCount,
        const char* pName = nullptr);

    RayTracingPipeline& addGeneralShaderGroup(const uint32_t shaderIndex);
    RayTracingPipeline& addHitShaderGroup(
        const uint32_t closestHitIndex, const uint32_t anyHitIndex, const uint32_t intersectionIndex);

    RayTracingPipeline& addDynamicState(const VkDynamicState dynamicState)
    {
        dynamicStates_.push_back(dynamicState);
        return *this;
    }

    template <typename T>
    RayTracingPipeline& addSpec(const size_t stageId, const T value)
    {
        static constexpr size_t size = sizeof(T);
        const char* data = (char*) &value;

        auto& info = moduleInfo_[id];
        for(size_t i = 0; i < size; i++)
        {
            info.specData.push_back(data[i]);
        }
        info.specSizes.push_back(size);

        return *this;
    }
    template <typename T, typename... Args>
    RayTracingPipeline& addSpec(const size_t stageId, const T value, Args&&... args)
    {
        addSpec<T>(stageId, value);
        return addSpec(stageId, std::forward<Args>(args)...);
    }

    bool createPipeline(
        const PipelineLayout& pipelineLayout, const uint32_t maxDepth,
        const VkPipelineCreateFlags flags = {});

    VkPipeline& getHandle() { return pipeline_; }
    const VkPipeline& getHandle() const { return pipeline_; }

    auto& dynamicStateInfo() { return dynamicStateInfo_; }
    const auto& dynamicStateInfo() const { return dynamicStateInfo_; }

  private:
    const Device* device_{nullptr};
    VkPipeline pipeline_{VK_NULL_HANDLE};

    std::vector<VkDynamicState> dynamicStates_ = {};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo_{};

    bool initialized_{false};

    struct ShaderStageInfo
    {
        VkShaderStageFlags shaderStage;
        VkShaderModule shaderModule{VK_NULL_HANDLE};
        std::string pName;
        std::vector<uint8_t> specData{};
        std::vector<size_t> specSizes{};
    };
    std::vector<ShaderStageInfo> moduleInfo_{};
    std::vector<std::vector<VkSpecializationMapEntry>> specMaps_;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages_{};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups_{};

    void finalizePipelineStages();
    void clearShaderModules();
};
} // namespace vkw
