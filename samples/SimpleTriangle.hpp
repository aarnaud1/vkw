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

class SimpleTriangle final : public IGraphicsSample
{
  public:
    SimpleTriangle();

    SimpleTriangle(const SimpleTriangle&) = delete;
    SimpleTriangle(SimpleTriangle&&) = delete;

    SimpleTriangle& operator=(const SimpleTriangle&) = delete;
    SimpleTriangle& operator=(SimpleTriangle&&) = delete;

    ~SimpleTriangle() {}

  private:
    static constexpr uint32_t imgWidth = 800;
    static constexpr uint32_t imgHeight = 600;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures_{};

    vkw::DeviceBuffer<glm::vec3> positions_{};
    vkw::DeviceBuffer<glm::vec3> colors_{};

    vkw::PipelineLayout pipelineLayout_{};
    vkw::GraphicsPipeline graphicsPipeline_{};

    VkPhysicalDevice findSupportedDevice() const override;

    bool init() override;
    bool recordInitCommands(vkw::CommandBuffer& cmdBuffer, const uint32_t frameId) override;
    void recordDrawCommands(
        vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId) override;
    bool recordPostDrawCommands(
        vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId) override;
    bool postDraw() override;
};