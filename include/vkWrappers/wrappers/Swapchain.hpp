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

#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Image.hpp"
#include "vkWrappers/wrappers/ImageView.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/RenderPass.hpp"
#include "vkWrappers/wrappers/RenderTarget.hpp"
#include "vkWrappers/wrappers/Synchronization.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class Swapchain
{
  public:
    Swapchain() {}

    explicit Swapchain(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t maxImageCount,
        const VkFormat colorFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    explicit Swapchain(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t maxImageCount,
        const VkFormat colorFormat,
        const VkFormat depthStencilFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    Swapchain(const Swapchain&) = delete;
    Swapchain(Swapchain&& cp) { *this = std::move(cp); }

    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain& operator=(Swapchain&& cp);
    ~Swapchain() { this->clear(); }

    bool init(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t maxImageCount,
        const VkFormat colorFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    bool init(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t maxImageCount,
        const VkFormat colorFormat,
        const VkFormat depthStencilFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    void clear();

    bool isInitialized() const { return initialized_; }

    VkResult getNextImage(uint32_t& imageIndex, Fence& fence, const uint64_t timeout);
    VkResult getNextImage(uint32_t& imageIndex, Semaphore& semaphore, const uint64_t timeout);
    VkResult getNextImage(
        uint32_t& imageIndex, Semaphore& semaphore, Fence& fence, const uint64_t timeout);

    uint32_t imageCount() const { return imageCount_; }

    VkSwapchainKHR& getHandle() { return swapchain_; }
    const VkSwapchainKHR& getHandle() const { return swapchain_; }

    VkExtent2D getExtent() const { return extent_; }

    VkFramebuffer& getFramebuffer(const size_t i) { return framebuffers_.at(i); }
    const VkFramebuffer& getFramebuffer(const size_t i) const { return framebuffers_.at(i); }

    auto& colorAttachment(const uint32_t i) { return colorAttachments_.at(i); }
    const auto& colorAttachment(const uint32_t i) const { return colorAttachments_.at(i); }

    auto& depthAttachment(const uint32_t i) { return depthStencilAttachments_.at(i); }
    const auto& depthAttachments(const uint32_t i) const { return depthStencilAttachments_.at(i); }

    bool reCreate(
        const uint32_t w,
        const uint32_t h,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});
    void clean(const bool clearSwapchain = true);

    VkFormat colorFormat() { return colorFormat_; }
    VkFormat depthStencilFormat() { return depthStencilFormat_; }

  private:
    Instance* instance_{nullptr};
    Device* device_{nullptr};
    RenderPass* renderPass_{nullptr};

    VkColorSpaceKHR colorSpace_{VK_COLOR_SPACE_MAX_ENUM_KHR};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};

    VkFormat colorFormat_{VK_FORMAT_UNDEFINED};
    VkFormat depthStencilFormat_{VK_FORMAT_UNDEFINED};

    VkImageUsageFlags usage_{0};
    uint32_t maxImageCount_{0};
    uint32_t imageCount_{0};
    std::vector<VkImage> images_{};

    bool useDepthStencil_{false};
    std::vector<ColorRenderTarget> colorAttachments_{};
    std::vector<DepthStencilRenderTarget> depthStencilAttachments_{};

    std::vector<VkFramebuffer> framebuffers_{};

    VkExtent2D extent_{};

    bool initialized_{false};

    bool create(
        const uint32_t w,
        const uint32_t h,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace,
        VkSharingMode sharingMode,
        const std::vector<uint32_t>& queueFamilyIndices,
        VkSwapchainKHR old);
    bool createImages();
    bool createFramebuffers();
};
} // namespace vkw