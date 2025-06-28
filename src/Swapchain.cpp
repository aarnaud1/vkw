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

#include "vkw/detail/Swapchain.hpp"

#include "vkw/detail/utils.hpp"

#include <limits>

namespace vkw
{
Swapchain::Swapchain(
    const Surface& surface,
    const Device& device,
    const RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    VKW_CHECK_BOOL_FAIL(
        this->init(
            surface,
            device,
            renderPass,
            w,
            h,
            maxImageCount,
            colorFormat,
            usage,
            colorSpace,
            sharingMode,
            queueFamilyIndices),
        "Creating swapchain");
}

Swapchain::Swapchain(
    const Surface& surface,
    const Device& device,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    VKW_CHECK_BOOL_FAIL(
        this->init(
            surface,
            device,
            w,
            h,
            maxImageCount,
            colorFormat,
            usage,
            colorSpace,
            sharingMode,
            queueFamilyIndices),
        "Creating swapchain");
}

Swapchain& Swapchain::operator=(Swapchain&& cp)
{
    this->clear();
    std::swap(surface_, cp.surface_);
    std::swap(device_, cp.device_);
    std::swap(renderPass_, cp.renderPass_);
    std::swap(swapchain_, cp.swapchain_);

    std::swap(colorFormat_, cp.colorFormat_);
    std::swap(imageViews_, cp.imageViews_);

    colorSpace_ = cp.colorSpace_;
    usage_ = cp.usage_;
    maxImageCount_ = cp.maxImageCount_;
    imageCount_ = cp.imageCount_;
    images_ = std::move(cp.images_);
    framebuffers_ = std::move(cp.framebuffers_);

    return *this;
}

bool Swapchain::init(
    const Surface& surface,
    const Device& device,
    const RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    VKW_ASSERT(this->initialized() == false);

    surface_ = &surface;
    device_ = &device;
    renderPass_ = &renderPass;

    usage_ = usage;
    colorFormat_ = colorFormat;
    maxImageCount_ = maxImageCount;

    VKW_INIT_CHECK_BOOL(
        this->create(w, h, usage, colorSpace, sharingMode, queueFamilyIndices, VK_NULL_HANDLE));
    initialized_ = true;

    return true;
}

bool Swapchain::init(
    const Surface& surface,
    const Device& device,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    VKW_ASSERT(this->initialized() == false);

    surface_ = &surface;
    device_ = &device;
    renderPass_ = nullptr;

    usage_ = usage;
    colorFormat_ = colorFormat;
    maxImageCount_ = maxImageCount;

    VKW_INIT_CHECK_BOOL(
        this->create(w, h, usage, colorSpace, sharingMode, queueFamilyIndices, VK_NULL_HANDLE));
    initialized_ = true;

    return true;
}

void Swapchain::clear()
{
    this->clean();

    surface_ = nullptr;
    device_ = nullptr;
    renderPass_ = nullptr;
    swapchain_ = VK_NULL_HANDLE;

    usage_ = 0;
    imageCount_ = 0;
    images_.clear();
    framebuffers_.clear();

    colorFormat_ = VK_FORMAT_UNDEFINED;
    imageViews_.clear();

    extent_ = {};

    initialized_ = false;
}

VkResult Swapchain::getNextImage(uint32_t& imageIndex, Fence& fence, const uint64_t timeout)
{
    return vkAcquireNextImageKHR(
        device_->getHandle(), swapchain_, timeout, VK_NULL_HANDLE, fence.getHandle(), &imageIndex);
}

VkResult Swapchain::getNextImage(uint32_t& imageIndex, Semaphore& semaphore, const uint64_t timeout)
{
    return device_->vk().vkAcquireNextImageKHR(
        device_->getHandle(), swapchain_, timeout, semaphore.getHandle(), VK_NULL_HANDLE, &imageIndex);
}

VkResult Swapchain::getNextImage(
    uint32_t& imageIndex, Semaphore& semaphore, Fence& fence, const uint64_t timeout)
{
    return device_->vk().vkAcquireNextImageKHR(
        device_->getHandle(), swapchain_, timeout, semaphore.getHandle(), fence.getHandle(), &imageIndex);
}

bool Swapchain::createImages()
{
    device_->vk().vkGetSwapchainImagesKHR(device_->getHandle(), swapchain_, &imageCount_, nullptr);

    images_.resize(imageCount_);
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkGetSwapchainImagesKHR(
        device_->getHandle(), swapchain_, &imageCount_, images_.data()));

    imageViews_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = images_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = colorFormat_;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.levelCount = 1;
        VKW_CHECK_BOOL_RETURN_FALSE(imageViews_[i].init(*device_, createInfo));
    }

    return true;
}

bool Swapchain::createFramebuffers()
{
    framebuffers_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        VkImageView imgView = imageViews_[i].getHandle();

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.renderPass = renderPass_->getHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &imgView;
        framebufferInfo.width = extent_.width;
        framebufferInfo.height = extent_.height;
        framebufferInfo.layers = 1;
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateFramebuffer(
            device_->getHandle(), &framebufferInfo, nullptr, &framebuffers_[i]));
    }

    return true;
}

void Swapchain::clean(const bool clearSwapchain)
{
    for(auto framebuffer : framebuffers_)
    {
        VKW_DELETE_VK(Framebuffer, framebuffer);
    }
    framebuffers_.clear();

    imageViews_.clear();
    images_.clear();

    if(clearSwapchain)
    {
        VKW_DELETE_VK(SwapchainKHR, swapchain_);
    }
}

bool Swapchain::reCreate(
    const uint32_t w,
    const uint32_t h,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    this->clean(false);
    if(!this->create(w, h, usage_, colorSpace_, sharingMode, queueFamilyIndices, swapchain_))
    {
        this->clean(false);
        return false;
    }
    return true;
}

bool Swapchain::create(
    const uint32_t w,
    const uint32_t h,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices,
    VkSwapchainKHR old)
{
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        device_->getPhysicalDevice(), surface_->getHandle(), &capabilities);

    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        extent_ = capabilities.currentExtent;
    }
    else
    {
        extent_ = {w, h};
    }

    colorSpace_ = colorSpace;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_->getHandle();
    createInfo.minImageCount = std::max(maxImageCount_, uint32_t(1));
    createInfo.imageFormat = colorFormat_;
    createInfo.imageColorSpace = colorSpace_;
    createInfo.imageExtent = extent_;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = usage;
    createInfo.imageSharingMode = sharingMode;
    createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
    createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = old;

    VKW_CHECK_VK_RETURN_FALSE(
        device_->vk().vkCreateSwapchainKHR(device_->getHandle(), &createInfo, nullptr, &swapchain_));

    if(old != VK_NULL_HANDLE)
    {
        device_->vk().vkDestroySwapchainKHR(device_->getHandle(), old, nullptr);
    }

    if(!createImages())
    {
        this->clean(true);
        return false;
    }

    if((swapchain_ != VK_NULL_HANDLE) && (renderPass_ != nullptr))
    {
        if(!createFramebuffers())
        {
            this->clean(true);
            return false;
        }
    }

    return true;
}
} // namespace vkw