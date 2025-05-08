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

#include "Common.hpp"

#include <vector>

struct GLFWwindow;

class IGraphicsSample
{
  public:
    IGraphicsSample();

    virtual ~IGraphicsSample();

    bool initSample();
    bool runSample();

    void requestResize() { needsResize_ = true; }

  protected:
    static constexpr uint32_t initWidth = 800;
    static constexpr uint32_t initHeight = 600;
    static constexpr uint32_t framesInFlight = 3;

    static constexpr VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    static constexpr VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    uint32_t frameWidth_{initWidth};
    uint32_t frameHeight_{initHeight};
    GLFWwindow* window_{nullptr};

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

    void handleResize();

  private:
    void initImageLayouts();
};