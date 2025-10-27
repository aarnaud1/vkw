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

#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Image.hpp"
#include "vkw/detail/ImageView.hpp"
#include "vkw/detail/RenderPass.hpp"
#include "vkw/detail/Surface.hpp"
#include "vkw/detail/Synchronization.hpp"

#include <vector>

namespace vkw
{
class Swapchain
{
  public:
    Swapchain() {}

    explicit Swapchain(
        const Surface& surface, const Device& device, const RenderPass& renderPass, const uint32_t w,
        const uint32_t h, const uint32_t maxImageCount, const VkFormat colorFormat,
        const VkImageUsageFlags usage, const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    explicit Swapchain(
        const Surface& surface, const Device& device, const uint32_t w, const uint32_t h,
        const uint32_t maxImageCount, const VkFormat colorFormat, const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    Swapchain(const Swapchain&) = delete;
    Swapchain(Swapchain&& cp) { *this = std::move(cp); }

    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain& operator=(Swapchain&& cp);
    ~Swapchain() { this->clear(); }

    bool init(
        const Surface& surface, const Device& device, const RenderPass& renderPass, const uint32_t w,
        const uint32_t h, const uint32_t maxImageCount, const VkFormat colorFormat,
        const VkImageUsageFlags usage, const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    bool init(
        const Surface& surface, const Device& device, const uint32_t w, const uint32_t h,
        const uint32_t maxImageCount, const VkFormat colorFormat, const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    void clear();

    bool initialized() const { return initialized_; }

    VkResult getNextImage(uint32_t& imageIndex, Fence& fence, const uint64_t timeout);
    VkResult getNextImage(uint32_t& imageIndex, Semaphore& semaphore, const uint64_t timeout);
    VkResult getNextImage(uint32_t& imageIndex, Semaphore& semaphore, Fence& fence, const uint64_t timeout);

    uint32_t imageCount() const { return imageCount_; }

    VkSwapchainKHR& getHandle() { return swapchain_; }
    const VkSwapchainKHR& getHandle() const { return swapchain_; }

    VkExtent2D getExtent() const { return extent_; }

    VkFramebuffer& getFramebuffer(const size_t i) { return framebuffers_.at(i); }
    const VkFramebuffer& getFramebuffer(const size_t i) const { return framebuffers_.at(i); }

    const auto& images() const { return images_; }
    const auto& imageViews() const { return imageViews_; }

    auto& imaeView(const uint32_t i) { return imageViews_.at(i); }
    const auto& imageView(const uint32_t i) const { return imageViews_.at(i); }

    bool reCreate(
        const uint32_t w, const uint32_t h, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});
    void clean(const bool clearSwapchain = true);

    VkFormat colorFormat() { return colorFormat_; }

  private:
    const Surface* surface_{nullptr};
    const Device* device_{nullptr};
    const RenderPass* renderPass_{nullptr};

    VkColorSpaceKHR colorSpace_{VK_COLOR_SPACE_MAX_ENUM_KHR};
    VkSwapchainKHR swapchain_{VK_NULL_HANDLE};

    VkFormat colorFormat_{VK_FORMAT_UNDEFINED};

    VkImageUsageFlags usage_{0};
    uint32_t maxImageCount_{0};
    uint32_t imageCount_{0};

    std::vector<VkImage> images_{};
    std::vector<ImageView> imageViews_{};
    std::vector<VkFramebuffer> framebuffers_{};

    VkExtent2D extent_{};

    bool initialized_{false};

    bool create(
        const uint32_t w, const uint32_t h, const VkImageUsageFlags usage, const VkColorSpaceKHR colorSpace,
        VkSharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices, VkSwapchainKHR old);
    bool createImages();
    bool createFramebuffers();
};
} // namespace vkw