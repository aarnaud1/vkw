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

#include "Common.hpp"

#include <vector>

class IGraphicsSample
{
  public:
    IGraphicsSample(
        const uint32_t frameWidth,
        const uint32_t frameHeight,
        const std::vector<const char*>& instanceExtensions);

    virtual ~IGraphicsSample();

    bool setSurface(VkSurfaceKHR&& surface);

    bool initSample();
    bool render();
    void finalize();

    void resize(const uint32_t w, const uint32_t h);

    const auto& instance() const { return instance_; }

  protected:
    static constexpr uint32_t framesInFlight = 3;
    static constexpr VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    static constexpr VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    uint32_t frameWidth_{0};
    uint32_t frameHeight_{0};

    std::vector<const char*> instanceLayers_{};
    std::vector<const char*> instanceExtensions_{};
    vkw::Instance instance_{};
    vkw::Surface surface_{};

    VkPhysicalDeviceFeatures2 deviceFeatures_{};
    std::vector<const char*> deviceExtensions_{};
    vkw::Device device_{};
    vkw::Queue graphicsQueue_{};
    vkw::Queue presentQueue_{};

    vkw::Swapchain swapchain_{};

    std::vector<vkw::Fence> frameFences_{};
    std::vector<vkw::Semaphore> imgSemaphores_{};
    std::vector<vkw::Semaphore> renderSemaphores_{};

    bool needsResize_{false};

    vkw::CommandPool cmdPool_{};
    std::vector<vkw::CommandBuffer> initCmdBuffers_{};
    std::vector<vkw::CommandBuffer> drawCmdBuffers_{};
    std::vector<vkw::CommandBuffer> postDrawCmdBuffers_{};

    virtual VkPhysicalDevice findSupportedDevice() const = 0;

    /// Init Vulkan resources for the sub class.
    virtual bool init() = 0;

    virtual bool recordInitCommands(vkw::CommandBuffer& cmdBuffer, const uint32_t frameId) = 0;
    virtual void recordDrawCommands(
        vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId)
        = 0;
    virtual bool recordPostDrawCommands(
        vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId)
        = 0;

    /// Use it to perform post draw operations - Synchronized with post draw commands execution.
    virtual bool postDraw() = 0;

  private:
    void initImageLayouts();
};