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

namespace vk
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
    const bool useDepth)
    : instance_{&instance}, device_{&device}, renderPass_{&renderPass}, useDepth_{useDepth}
{
    create(w, h, imageFormat, usage, colorSpace);
}

Swapchain::Swapchain(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat imageFormat,
    const VkColorSpaceKHR colorSpace,
    const bool useDepth)
    : Swapchain(
        instance,
        device,
        renderPass,
        w,
        h,
        imageFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        colorSpace,
        useDepth)
{}

Swapchain::~Swapchain() { clean(); }

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

    imageViews_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        CHECK_VK(
            vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageViews_[i]),
            "Creating image view");
    }

    if(useDepth_)
    {
        depthStencilMemory_.reset(new Memory(*device_, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        depthStencilImages_.resize(imageCount_);
        for(size_t i = 0; i < imageCount_; ++i)
        {
            depthStencilImages_[i]
                = &depthStencilMemory_->createImage<ImageFormat::DEPTH_24_STENCIL_8, uint32_t>(
                    VK_IMAGE_TYPE_2D,
                    VkExtent3D{extent_.width, extent_.height, 1},
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    1,
                    VK_IMAGE_TILING_OPTIMAL,
                    1);
        }
        depthStencilMemory_->allocate();

        depthStencilImageViews_.resize(imageCount_);
        for(size_t i = 0; i < imageCount_; ++i)
        {
            depthStencilImageViews_[i]
                = ImageView<Image<ImageFormat::DEPTH_24_STENCIL_8, uint32_t>>(
                    *device_,
                    *depthStencilImages_[i],
                    VK_IMAGE_VIEW_TYPE_2D,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    {VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1});
        }
    }
}

void Swapchain::createFramebuffers()
{
    framebuffers_.resize(imageCount_);
    for(size_t i = 0; i < imageCount_; ++i)
    {
        VkImageView attachments[] = {imageViews_[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass_->getHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
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
    const VkFormat imageFormat,
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
    createInfo.minImageCount = 8;
    createInfo.imageFormat = imageFormat;
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
    const VkFormat imageFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace)
{
    clean(false);
    create(w, h, imageFormat, usage, colorSpace, swapchain_);
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
        for(auto& imageView : imageViews_)
        {
            if(imageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device_->getHandle(), imageView, nullptr);
            }
        }
        if(useDepth_)
        {
            depthStencilImageViews_.clear();
            depthStencilImages_.clear();
            depthStencilMemory_.reset(nullptr);
        }
        images_.clear();
        imageViews_.clear();
        framebuffers_.clear();

        if(clearSwapchain)
        {
            vkDestroySwapchainKHR(device_->getHandle(), swapchain_, nullptr);
            swapchain_ = nullptr;
        }
    }
}

} // namespace vk