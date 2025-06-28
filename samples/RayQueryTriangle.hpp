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

#include "IGraphicsSample.hpp"

#include <glm/glm.hpp>
#include <vkw/high_level/Types.hpp>

class RayQueryTriangle final : public IGraphicsSample
{
  public:
    RayQueryTriangle() = delete;
    RayQueryTriangle(
        const uint32_t fboWidth,
        const uint32_t fboHeight,
        const std::vector<const char*>& instanceLayers,
        const std::vector<const char*>& instanceExtensions);

    RayQueryTriangle(const RayQueryTriangle&) = delete;
    RayQueryTriangle(RayQueryTriangle&&) = delete;

    RayQueryTriangle& operator=(const RayQueryTriangle&) = delete;
    RayQueryTriangle& operator=(RayQueryTriangle&&) = delete;

    ~RayQueryTriangle() {}

  private:
    using GeometryType
        = vkw::AccelerationStructureTriangleData<VK_FORMAT_R32G32B32_SFLOAT, VK_INDEX_TYPE_UINT32>;

    ///@todo: Use a more complex model
    static constexpr uint32_t vertexCount = 3;
    static constexpr uint32_t triangleCount = 1;

    static inline const glm::vec3 triangleData[3]
        = {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
    static inline const VkTransformMatrixKHR transform{vkw::asIdentityMatrix};
    static inline const uint32_t indices[3 * triangleCount] = {0, 1, 2};

    uint32_t fboWidth_{};
    uint32_t fboHeight_{};

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures_{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR deviceAccelerationStructureFeatures_{};
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddressFeatures_{};

    struct PushConstants
    {
        uint32_t sizeX;
        uint32_t sizeY;
    };

    vkw::DescriptorSetLayout descriptorSetLayout_{};
    vkw::PipelineLayout pipelineLayout_{};
    vkw::ComputePipeline pipeline_{};

    vkw::DescriptorPool descriptorPool_{};
    std::vector<vkw::DescriptorSet> descriptorSets_{};

    vkw::AccelerationStructureGeometryBuffer<glm::vec3> vertexBuffer_{};
    vkw::AccelerationStructureGeometryBuffer<uint32_t> indexBuffer_{};
    vkw::AccelerationStructureGeometryBuffer<VkTransformMatrixKHR> transformBuffer_{};

    GeometryType geometryData_{};
    vkw::BottomLevelAccelerationStructure bottomLevelAs_{};
    vkw::TopLevelAccelerationStructure topLevelAs_{};

    vkw::AccelerationStructureSratchBuffer scratchBuffer_{};
    std::vector<vkw::DeviceImage<>> outputImages_{};
    std::vector<vkw::ImageView> outputImagesViews_{};

    VkPhysicalDevice findSupportedDevice() const override;

    bool init() override;
    bool recordInitCommands(vkw::CommandBuffer& cmdBuffer, const uint32_t frameId) override;
    void recordDrawCommands(
        vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId) override;
    bool recordPostDrawCommands(
        vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId) override;
    bool postDraw() override;
};