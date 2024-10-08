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

#include "vkWrappers/wrappers/Swapchain.hpp"

#include <limits>

namespace vkw
{
Swapchain::Swapchain(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat imageFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    const bool useDepth,
    const VkFormat depthFormat)
{
    this->init(
        instance, device, renderPass, w, h, imageFormat, usage, colorSpace, useDepth, depthFormat);
}

Swapchain::Swapchain(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat imageFormat,
    const VkColorSpaceKHR colorSpace,
    const bool useDepth,
    const VkFormat depthFormat)
    : Swapchain(
          instance,
          device,
          renderPass,
          w,
          h,
          imageFormat,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
          colorSpace,
          useDepth,
          depthFormat)
{}

void Swapchain::init(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat imageFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    const bool useDepth,
    const VkFormat depthFormat)
{
    if(!initialized_)
    {
        instance_ = &instance;
        device_ = &device;
        renderPass_ = &renderPass;
        useDepth_ = useDepth;

        colorFormat_ = imageFormat;
        depthStencilFormat_ = depthFormat;

        this->create(w, h, usage, colorSpace);
        initialized_ = true;
    }
}

void Swapchain::init(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat imageFormat,
    const VkColorSpaceKHR colorSpace,
    const bool useDepth,
    const VkFormat depthFormat)
{
    this->init(
        instance,
        device,
        renderPass,
        w,
        h,
        imageFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        colorSpace,
        useDepth,
        depthFormat);
}

void Swapchain::clear()
{
    this->clean();

    instance_ = nullptr;
    device_ = nullptr;
    renderPass_ = nullptr;
    swapchain_ = VK_NULL_HANDLE;

    imageCount_ = 0;
    images_.clear();
    framebuffers_.clear();

    useDepth_ = true;
    colorAttachments_.clear();
    depthStencilAttachments_.clear();

    extent_ = {};

    initialized_ = false;
}

VkResult Swapchain::getNextImage(uint32_t& imageIndex)
{
    return vkAcquireNextImageKHR(
        device_->getHandle(),
        swapchain_,
        std::numeric_limits<uint64_t>::max(),
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        &imageIndex);
}

VkResult Swapchain::getNextImage(uint32_t& imageIndex, Semaphore& semaphore)
{
    return vkAcquireNextImageKHR(
        device_->getHandle(),
        swapchain_,
        std::numeric_limits<uint64_t>::max(),
        semaphore.getHandle(),
        VK_NULL_HANDLE,
        &imageIndex);
}

void Swapchain::createImages()
{
    vkGetSwapchainImagesKHR(device_->getHandle(), swapchain_, &imageCount_, nullptr);

    images_.resize(imageCount_);
    vkGetSwapchainImagesKHR(device_->getHandle(), swapchain_, &imageCount_, images_.data());

    colorAttachments_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        colorAttachments_[i].init(
            *device_, extent_.width, extent_.height, colorFormat_, images_[i]);
    }

    if(useDepth_)
    {
        depthStencilAttachments_.resize(imageCount_);
        for(size_t i = 0; i < imageCount_; ++i)
        {
            depthStencilAttachments_[i].init(
                *device_, extent_.width, extent_.height, depthStencilFormat_);
        }
    }
}

void Swapchain::createFramebuffers()
{
    framebuffers_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        std::vector<VkImageView> attachments{colorAttachments_[i].imageView()};
        if(useDepth_)
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

        CHECK_VK(
            vkCreateFramebuffer(device_->getHandle(), &framebufferInfo, nullptr, &framebuffers_[i]),
            "Creating framebuffer");
    }
}

void Swapchain::create(
    const uint32_t w,
    const uint32_t h,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace,
    VkSwapchainKHR old)
{
    const uint32_t indices[2]
        = {device_->getQueueFamilies().getQueueFamilyIndex<QueueFamilyType::GRAPHICS>(),
           device_->getQueueFamilies().getQueueFamilyIndex<QueueFamilyType::PRESENT>()};
    const bool equalIndices = (indices[0] == indices[1]);

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

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = instance_->getSurface();
    createInfo.minImageCount = 4;
    createInfo.imageFormat = colorFormat_;
    createInfo.imageColorSpace = colorSpace;
    createInfo.imageExtent = extent_;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = usage;
    createInfo.imageSharingMode
        = equalIndices ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = equalIndices ? 0 : 2;
    createInfo.pQueueFamilyIndices = equalIndices ? nullptr : indices;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = old;

    CHECK_VK(
        vkCreateSwapchainKHR(device_->getHandle(), &createInfo, nullptr, &swapchain_),
        "Creating swapchain");

    if(old != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device_->getHandle(), old, nullptr);
    }

    createImages();
    createFramebuffers();
}

void Swapchain::reCreate(
    const uint32_t w,
    const uint32_t h,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace)
{
    this->clean(false);
    this->create(w, h, usage, colorSpace, swapchain_);
}

void Swapchain::clean(const bool clearSwapchain)
{
    if(swapchain_ != VK_NULL_HANDLE)
    {
        for(auto& framebuffer : framebuffers_)
        {
            if(framebuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device_->getHandle(), framebuffer, nullptr);
            }
        }
        colorAttachments_.clear();
        depthStencilAttachments_.clear();
        images_.clear();
        framebuffers_.clear();

        if(clearSwapchain)
        {
            vkDestroySwapchainKHR(device_->getHandle(), swapchain_, nullptr);
            swapchain_ = nullptr;
        }
    }
}

} // namespace vkw