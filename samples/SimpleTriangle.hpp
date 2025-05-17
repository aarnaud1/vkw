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