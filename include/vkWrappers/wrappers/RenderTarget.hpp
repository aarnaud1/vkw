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
#include "vkWrappers/wrappers/Memory.hpp"

#include <vulkan/vulkan.h>

namespace vkw
{
class RenderTarget
{
  public:
    RenderTarget() : targetId_{nextTargetId++} {}

    RenderTarget(const RenderTarget&) = delete;
    RenderTarget(RenderTarget&& cp) : targetId_{cp.targetId_} { *this = std::move(cp); }

    RenderTarget& operator=(const RenderTarget&) = delete;
    RenderTarget& operator=(RenderTarget&& cp)
    {
        this->clear();

        std::swap(device_, cp.device_);
        std::swap(externalImage_, cp.externalImage_);
        std::swap(image_, cp.image_);
        std::swap(imageView_, cp.imageView_);
        std::swap(imageSampler_, cp.imageSampler_);
        std::swap(imageMemory_, cp.imageMemory_);

        std::swap(extent_, cp.extent_);

        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    virtual ~RenderTarget() { this->clear(); }

    inline bool isInitialized() const { return initialized_; }
    inline VkImageView imageView() const { return imageView_; }
    inline VkExtent2D extent() const { return extent_; }
    inline VkSampler sampler() const { return imageSampler_; }

    auto& image() { return *image_; }
    const auto& image() const { return *image_; }

    uint32_t getId() const { return targetId_; }

    virtual void init(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat format,
        VkImage img = VK_NULL_HANDLE)
        = 0;

    void clear()
    {
        imageMemory_.clear();

        if(imageView_ != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device_->getHandle(), imageView_, nullptr);
        }

        if(imageSampler_ != VK_NULL_HANDLE)
        {
            vkDestroySampler(device_->getHandle(), imageSampler_, nullptr);
        }

        device_ = nullptr;
        externalImage_ = VK_NULL_HANDLE;
        imageView_ = VK_NULL_HANDLE;
        imageSampler_ = VK_NULL_HANDLE;

        initialized_ = false;
    }

  protected:
    static inline uint32_t nextTargetId = 0;
    const uint32_t targetId_;

    Device* device_{nullptr};

    VkImage externalImage_{VK_NULL_HANDLE};
    VkImageView imageView_{VK_NULL_HANDLE};
    VkSampler imageSampler_{VK_NULL_HANDLE};

    Memory imageMemory_{};
    Image* image_{nullptr};

    VkExtent2D extent_{};

    bool initialized_{false};
};

class ColorRenderTarget final : public RenderTarget
{
  public:
    ColorRenderTarget() {}
    ColorRenderTarget(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imgFormat = VK_FORMAT_B8G8R8A8_SRGB,
        VkImage img = VK_NULL_HANDLE)
    {
        this->init(device, w, h, imgFormat, img);
    }

    void setLoadPolicy(const VkAttachmentLoadOp op) { loadPolicy_ = op; }
    void setStorePolicy(const VkAttachmentStoreOp op) { storePolicy_ = op; }

    auto getLoadPolicy() const { return loadPolicy_; }
    auto getStorePolicy() const { return storePolicy_; }

    auto format() const { return format_; }

    void init(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imgFormat = VK_FORMAT_B8G8R8A8_SRGB,
        VkImage img = VK_NULL_HANDLE) override
    {
        if(!initialized_)
        {
            device_ = &device;
            externalImage_ = img;
            format_ = imgFormat;

            if(externalImage_ == VK_NULL_HANDLE)
            {
                imageMemory_.init(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                image_ = &imageMemory_.createImage(
                    VK_IMAGE_TYPE_2D,
                    imgFormat,
                    VkExtent3D{w, h, 1},
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
                imageMemory_.allocate();
            }

            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image
                = (externalImage_ != VK_NULL_HANDLE) ? externalImage_ : image_->getHandle();
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imgFormat;
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
                vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageView_),
                "Creating color attachment image view");

            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.pNext = nullptr;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.maxAnisotropy = 1.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 1.0f;
            samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            CHECK_VK(
                vkCreateSampler(device_->getHandle(), &samplerInfo, nullptr, &imageSampler_),
                "Creating color attachment sampler");

            extent_.width = w;
            extent_.height = h;

            initialized_ = true;
        }
    }

  private:
    VkFormat format_{};
    VkAttachmentLoadOp loadPolicy_{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
    VkAttachmentStoreOp storePolicy_{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};

class DepthRenderTarget final : public RenderTarget
{
  public:
    DepthRenderTarget() {}
    DepthRenderTarget(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT,
        VkImage img = VK_NULL_HANDLE)
    {
        this->init(device, w, h, depthStencilFormat, img);
    }

    void setDepthLoadPolicy(const VkAttachmentLoadOp op) { depthLoadPolicy_ = op; }
    void setDepthStorePolicy(const VkAttachmentStoreOp op) { depthStorePolicy_ = op; }
    void setStencilLoadPolicy(const VkAttachmentLoadOp op) { stencilLoadPolicy_ = op; }
    void setStencilStorePolicy(const VkAttachmentStoreOp op) { stencilStorePolicy_ = op; }

    auto getDepthLoadPolicy() const { return depthLoadPolicy_; }
    auto getDepthStorePolicy() const { return depthStorePolicy_; }
    auto getStencilLoadPolicy() const { return stencilLoadPolicy_; }
    auto getStencilStorePolicy() const { return stencilStorePolicy_; }

    auto format() const { return format_; }

    void init(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT,
        VkImage img = VK_NULL_HANDLE) override
    {
        if(!initialized_)
        {
            device_ = &device;
            externalImage_ = img;
            format_ = depthStencilFormat;

            if(externalImage_ == VK_NULL_HANDLE)
            {
                imageMemory_.init(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                image_ = &imageMemory_.createImage(
                    VK_IMAGE_TYPE_2D,
                    depthStencilFormat,
                    VkExtent3D{w, h, 1},
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
                imageMemory_.allocate();
            }

            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image
                = (externalImage_ != VK_NULL_HANDLE) ? externalImage_ : image_->getHandle();
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = depthStencilFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask
                = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            CHECK_VK(
                vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageView_),
                "Creating depth stencil attachment image view");

            extent_.width = w;
            extent_.height = h;

            initialized_ = true;
        }
    }

  private:
    VkFormat format_{};

    VkAttachmentLoadOp depthLoadPolicy_{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
    VkAttachmentStoreOp depthStorePolicy_{VK_ATTACHMENT_STORE_OP_DONT_CARE};

    VkAttachmentLoadOp stencilLoadPolicy_{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
    VkAttachmentStoreOp stencilStorePolicy_{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};
} // namespace vkw