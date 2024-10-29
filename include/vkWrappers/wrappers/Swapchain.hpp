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

#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Image.hpp"
#include "vkWrappers/wrappers/ImageView.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/Memory.hpp"
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
        const VkFormat imageFormat,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        const bool useDepth = true,
        const VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT);

    explicit Swapchain(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        const bool useDepth = true,
        const VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT);

    Swapchain(const Swapchain&) = delete;
    Swapchain(Swapchain&& cp) { *this = std::move(cp); }

    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain& operator=(Swapchain&& cp)
    {
        this->clear();
        std::swap(instance_, cp.instance_);
        std::swap(device_, cp.device_);
        std::swap(renderPass_, cp.renderPass_);
        std::swap(swapchain_, cp.swapchain_);

        std::swap(colorFormat_, cp.colorFormat_);
        std::swap(depthStencilFormat_, cp.depthStencilFormat_);

        std::swap(colorAttachments_, cp.colorAttachments_);
        std::swap(depthStencilAttachments_, cp.depthStencilAttachments_);

        imageCount_ = cp.imageCount_;
        images_ = std::move(cp.images_);
        framebuffers_ = std::move(cp.framebuffers_);

        return *this;
    }

    ~Swapchain() { this->clear(); }

    void init(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        const bool useDepth = true,
        const VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT);

    void init(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        const bool useDepth = true,
        const VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkResult getNextImage(uint32_t& imageIndex, const uint64_t timeout);
    VkResult getNextImage(uint32_t& imageIndex, Semaphore& semaphore, const uint64_t timeout);

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

    void create(
        const uint32_t w,
        const uint32_t h,
        const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSwapchainKHR old = VK_NULL_HANDLE,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});
    void reCreate(
        const uint32_t w,
        const uint32_t h,
        const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    void clean(const bool clearSwapchain = true);

    VkFormat colorFormat() { return colorFormat_; }
    VkFormat depthStencilFormat() { return depthStencilFormat_; }

  private:
    Instance* instance_{nullptr};
    Device* device_{nullptr};
    RenderPass* renderPass_{nullptr};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};

    VkFormat colorFormat_;
    VkFormat depthStencilFormat_;

    uint32_t imageCount_{0};
    std::vector<VkImage> images_{};

    bool useDepth_{true};
    std::vector<ColorRenderTarget> colorAttachments_{};
    std::vector<DepthRenderTarget> depthStencilAttachments_{};

    std::vector<VkFramebuffer> framebuffers_{};

    VkExtent2D extent_{};

    bool initialized_{false};

    void createImages();

    void createFramebuffers();
};
} // namespace vkw