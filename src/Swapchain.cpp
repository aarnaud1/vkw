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

#include "vkWrappers/wrappers/Swapchain.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#include <limits>

namespace vkw
{
Swapchain::Swapchain(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    VKW_CHECK_BOOL_THROW(
        this->init(
            instance,
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
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkFormat depthStencilFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    VKW_CHECK_BOOL_THROW(
        this->init(
            instance,
            device,
            renderPass,
            w,
            h,
            maxImageCount,
            colorFormat,
            depthStencilFormat,
            usage,
            colorSpace,
            sharingMode,
            queueFamilyIndices),
        "Creating swapchain");
}

Swapchain& Swapchain::operator=(Swapchain&& cp)
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

    colorSpace_ = cp.colorSpace_;
    usage_ = cp.usage_;
    maxImageCount_ = cp.maxImageCount_;
    imageCount_ = cp.imageCount_;
    images_ = std::move(cp.images_);
    framebuffers_ = std::move(cp.framebuffers_);

    return *this;
}

bool Swapchain::init(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    if(!initialized_)
    {
        instance_ = &instance;
        device_ = &device;
        renderPass_ = &renderPass;

        usage_ = usage;
        useDepthStencil_ = false;
        colorFormat_ = colorFormat;
        depthStencilFormat_ = VK_FORMAT_UNDEFINED;
        maxImageCount_ = maxImageCount;

        VKW_INIT_CHECK_BOOL(
            this->create(w, h, usage, colorSpace, sharingMode, queueFamilyIndices, VK_NULL_HANDLE));
        initialized_ = true;
    }

    return true;
}

bool Swapchain::init(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const uint32_t maxImageCount,
    const VkFormat colorFormat,
    const VkFormat depthStencilFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSharingMode sharingMode,
    const std::vector<uint32_t>& queueFamilyIndices)
{
    if(!initialized_)
    {
        instance_ = &instance;
        device_ = &device;
        renderPass_ = &renderPass;

        usage_ = usage;
        useDepthStencil_ = true;
        colorFormat_ = colorFormat;
        depthStencilFormat_ = depthStencilFormat;
        maxImageCount_ = maxImageCount;

        VKW_INIT_CHECK_BOOL(
            this->create(w, h, usage, colorSpace, sharingMode, queueFamilyIndices, VK_NULL_HANDLE));
        initialized_ = true;
    }

    return true;
}

void Swapchain::clear()
{
    this->clean();

    instance_ = nullptr;
    device_ = nullptr;
    renderPass_ = nullptr;
    swapchain_ = VK_NULL_HANDLE;

    usage_ = 0;
    imageCount_ = 0;
    images_.clear();
    framebuffers_.clear();

    useDepthStencil_ = false;
    colorFormat_ = VK_FORMAT_UNDEFINED;
    depthStencilFormat_ = VK_FORMAT_UNDEFINED;

    colorAttachments_.clear();
    depthStencilAttachments_.clear();

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
        device_->getHandle(),
        swapchain_,
        timeout,
        semaphore.getHandle(),
        VK_NULL_HANDLE,
        &imageIndex);
}

VkResult Swapchain::getNextImage(
    uint32_t& imageIndex, Semaphore& semaphore, Fence& fence, const uint64_t timeout)
{
    return device_->vk().vkAcquireNextImageKHR(
        device_->getHandle(),
        swapchain_,
        timeout,
        semaphore.getHandle(),
        fence.getHandle(),
        &imageIndex);
}

bool Swapchain::createImages()
{
    device_->vk().vkGetSwapchainImagesKHR(device_->getHandle(), swapchain_, &imageCount_, nullptr);

    images_.resize(imageCount_);
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkGetSwapchainImagesKHR(
        device_->getHandle(), swapchain_, &imageCount_, images_.data()));

    colorAttachments_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        colorAttachments_[i].init(
            *device_, extent_.width, extent_.height, colorFormat_, {}, images_[i]);
    }

    if(useDepthStencil_)
    {
        depthStencilAttachments_.resize(imageCount_);
        for(size_t i = 0; i < imageCount_; ++i)
        {
            depthStencilAttachments_[i].init(
                *device_, extent_.width, extent_.height, depthStencilFormat_);
        }
    }

    return true;
}

bool Swapchain::createFramebuffers()
{
    framebuffers_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        std::vector<VkImageView> attachments{colorAttachments_[i].imageView()};
        if(useDepthStencil_)
        {
            attachments.emplace_back(depthStencilAttachments_[i].imageView());
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.renderPass = renderPass_->getHandle();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
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

    colorAttachments_.clear();
    depthStencilAttachments_.clear();
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
        device_->getPhysicalDevice(), instance_->getSurface(), &capabilities);

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
    createInfo.surface = instance_->getSurface();
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

    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkCreateSwapchainKHR(
        device_->getHandle(), &createInfo, nullptr, &swapchain_));

    if(old != VK_NULL_HANDLE)
    {
        device_->vk().vkDestroySwapchainKHR(device_->getHandle(), old, nullptr);
    }

    if(!createImages())
    {
        this->clean(true);
        return false;
    }

    if(swapchain_ != VK_NULL_HANDLE)
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