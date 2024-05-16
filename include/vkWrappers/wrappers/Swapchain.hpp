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

#include <vulkan/vulkan.h>

#include <vector>

#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/RenderPass.hpp"
#include "vkWrappers/wrappers/Synchronization.hpp"

namespace vk
{
class Swapchain
{
  public:
    explicit Swapchain(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

    explicit Swapchain(
        Instance& instance,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

    Swapchain(const Swapchain&) = delete;
    Swapchain(Swapchain&& cp)
    {
        clean();
        std::swap(instance_, cp.instance_);
        std::swap(device_, cp.device_);
        std::swap(renderPass_, cp.renderPass_);
        std::swap(swapchain_, cp.swapchain_);

        imageCount_ = cp.imageCount_;
        images_ = std::move(cp.images_);
        imageViews_ = std::move(cp.imageViews_);
        framebuffers_ = std::move(cp.framebuffers_);
    }

    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain& operator=(Swapchain&& cp)
    {
        clean();
        std::swap(instance_, cp.instance_);
        std::swap(device_, cp.device_);
        std::swap(renderPass_, cp.renderPass_);
        std::swap(swapchain_, cp.swapchain_);

        imageCount_ = cp.imageCount_;
        images_ = std::move(cp.images_);
        imageViews_ = std::move(cp.imageViews_);
        framebuffers_ = std::move(cp.framebuffers_);

        return *this;
    }

    ~Swapchain();

    VkResult getNextImage(uint32_t& imageIndex);
    VkResult getNextImage(uint32_t& imageIndex, Semaphore& semaphore);

    uint32_t imageCount() const { return imageCount_; }

    VkSwapchainKHR& getHandle() { return swapchain_; }
    const VkSwapchainKHR& getHandle() const { return swapchain_; }

    VkExtent2D getExtent() const { return extent_; }

    void addDepthStencilImages(const size_t count, const VkFormat = VK_FORMAT_D32_SFLOAT);

    VkFramebuffer& getFramebuffer(const size_t i) { return framebuffers_.at(i); }
    const VkFramebuffer& getFramebuffer(const size_t i) const { return framebuffers_.at(i); }

    void create(
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSwapchainKHR old = VK_NULL_HANDLE);
    void reCreate(
        const uint32_t w,
        const uint32_t h,
        const VkFormat imageFormat,
        const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    void clean(const bool clearSwapchain = true);

  private:
    Instance* instance_{nullptr};
    Device* device_{nullptr};
    RenderPass* renderPass_{nullptr};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};

    uint32_t imageCount_{0};
    std::vector<VkImage> images_{};
    std::vector<VkImageView> imageViews_{};
    std::vector<VkFramebuffer> framebuffers_{};

    VkExtent2D extent_{};

    void createImages();

    void createFramebuffers();
};
} // namespace vk