/*
 * Copyright (C) 2025 Adrien ARNAUD
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

#include "IGraphicsSample.hpp"

#include <glm/glm.hpp>

class RayQueryTriangle final : public IGraphicsSample
{
  public:
    RayQueryTriangle();

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

    vkw::DeviceBuffer<glm::vec3> vertexBuffer_{};
    vkw::DeviceBuffer<uint32_t> indexBuffer_{};
    ///@todo: Check if glm::mat4 is compatible with VkTransformMatrix
    vkw::DeviceBuffer<VkTransformMatrixKHR> transformBuffer_{};

    GeometryType geometryData_{};
    vkw::BottomLevelAccelerationStructure bottomLevelAs_{};
    vkw::TopLevelAccelerationStructure topLevelAs_{};

    vkw::HostBuffer<uint8_t> scratchBuffer_{};

    std::vector<vkw::DeviceImage> outputImages_{};
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