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

#include <vulkan/vulkan.h>

namespace vkw
{
class RenderTarget
{
  public:
    RenderTarget() {}

    RenderTarget(const RenderTarget&) = delete;
    RenderTarget(RenderTarget&& rhs) { *this = std::move(rhs); }

    RenderTarget& operator=(const RenderTarget&) = delete;
    RenderTarget& operator=(RenderTarget&& rhs)
    {
        this->clear();

        std::swap(device_, rhs.device_);
        std::swap(externalImage_, rhs.externalImage_);
        std::swap(image_, rhs.image_);
        std::swap(imageView_, rhs.imageView_);
        std::swap(imageSampler_, rhs.imageSampler_);

        std::swap(extent_, rhs.extent_);
        std::swap(clearValue_, rhs.clearValue_);

        std::swap(initialized_, rhs.initialized_);

        return *this;
    }

    virtual ~RenderTarget() { this->clear(); }

    inline bool isInitialized() const { return initialized_; }

    inline auto imageView() const { return imageView_; }
    inline auto extent() const { return extent_; }
    inline auto sampler() const { return imageSampler_; }
    inline auto clearValue() const { return clearValue_; }

    auto& image() { return image_; }
    const auto& image() const { return image_; }

    auto externalImage() const { return externalImage_; }

    virtual bool init(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat format,
        const VkClearValue clearValue = {},
        VkImage img = VK_NULL_HANDLE)
        = 0;

    void clear()
    {
        VKW_DELETE_VK(ImageView, imageView_);
        VKW_DELETE_VK(Sampler, imageSampler_);
        externalImage_ = VK_NULL_HANDLE;

        image_.clear();

        device_ = nullptr;
        initialized_ = false;
    }

  protected:
    Device* device_{nullptr};

    VkImage externalImage_{VK_NULL_HANDLE};
    VkImageView imageView_{VK_NULL_HANDLE};
    VkSampler imageSampler_{VK_NULL_HANDLE};

    DeviceImage image_{};

    VkExtent2D extent_{};
    VkClearValue clearValue_{};

    bool initialized_{false};
};

class ColorRenderTarget final : public RenderTarget
{
  public:
    ColorRenderTarget() {}

    explicit ColorRenderTarget(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imgFormat,
        const VkClearValue clearValue = {},
        VkImage img = VK_NULL_HANDLE)
    {
        VKW_CHECK_BOOL_THROW(
            this->init(device, w, h, imgFormat, clearValue, img), "Creating color render target");
    }

    ColorRenderTarget(const ColorRenderTarget&) = delete;
    ColorRenderTarget(ColorRenderTarget&& rhs) { *this = std::move(rhs); }

    ColorRenderTarget& operator=(const ColorRenderTarget&) = delete;
    ColorRenderTarget& operator=(ColorRenderTarget&& rhs)
    {
        RenderTarget::operator=(std::move(rhs));

        std::swap(format_, rhs.format_);
        std::swap(loadPolicy_, rhs.loadPolicy_);
        std::swap(storePolicy_, rhs.storePolicy_);

        return *this;
    }

    void setLoadPolicy(const VkAttachmentLoadOp op) { loadPolicy_ = op; }
    void setStorePolicy(const VkAttachmentStoreOp op) { storePolicy_ = op; }

    auto loadOp() const { return loadPolicy_; }
    auto storeOp() const { return storePolicy_; }

    auto format() const { return format_; }

    bool init(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat imgFormat = VK_FORMAT_B8G8R8A8_SRGB,
        const VkClearValue clearValue = {},
        VkImage img = VK_NULL_HANDLE) override
    {
        if(!initialized_)
        {
            device_ = &device;
            externalImage_ = img;
            format_ = imgFormat;
            clearValue_.color = clearValue.color;

            if(externalImage_ == VK_NULL_HANDLE)
            {
                VKW_INIT_CHECK_BOOL(image_.init(
                    device,
                    VK_IMAGE_TYPE_2D,
                    imgFormat,
                    VkExtent3D{w, h, 1},
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT));
            }

            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image
                = (externalImage_ != VK_NULL_HANDLE) ? externalImage_ : image_.getHandle();
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
            VKW_INIT_CHECK_VK(
                vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageView_));

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
            VKW_INIT_CHECK_VK(
                vkCreateSampler(device_->getHandle(), &samplerInfo, nullptr, &imageSampler_));

            extent_.width = w;
            extent_.height = h;

            initialized_ = true;
        }

        return true;
    }

  private:
    VkFormat format_{};
    VkAttachmentLoadOp loadPolicy_{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
    VkAttachmentStoreOp storePolicy_{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};

class DepthStencilRenderTarget final : public RenderTarget
{
  public:
    DepthStencilRenderTarget() {}

    explicit DepthStencilRenderTarget(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat depthStencilFormat,
        const VkClearValue clearValue = {},
        VkImage img = VK_NULL_HANDLE)
    {
        VKW_CHECK_BOOL_THROW(
            this->init(device, w, h, depthStencilFormat, clearValue, img),
            "Creating depth render target");
    }

    DepthStencilRenderTarget(const DepthStencilRenderTarget&) = delete;
    DepthStencilRenderTarget(DepthStencilRenderTarget&& rhs) { *this = std::move(rhs); }

    DepthStencilRenderTarget& operator=(const DepthStencilRenderTarget&) = delete;
    DepthStencilRenderTarget& operator=(DepthStencilRenderTarget&& rhs)
    {
        RenderTarget::operator=(std::move(rhs));

        std::swap(format_, rhs.format_);
        std::swap(loadPolicy_, rhs.loadPolicy_);
        std::swap(storePolicy_, rhs.storePolicy_);

        return *this;
    }

    void setLoadPolicy(const VkAttachmentLoadOp op) { loadPolicy_ = op; }
    void setStorePolicy(const VkAttachmentStoreOp op) { storePolicy_ = op; }

    auto loadOp() const { return loadPolicy_; }
    auto storeOp() const { return storePolicy_; }

    auto format() const { return format_; }

    bool init(
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const VkFormat depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT,
        const VkClearValue clearValue = {},
        VkImage img = VK_NULL_HANDLE) override
    {
        if(!initialized_)
        {
            device_ = &device;
            externalImage_ = img;
            format_ = depthStencilFormat;
            clearValue_.depthStencil = clearValue.depthStencil;

            if(externalImage_ == VK_NULL_HANDLE)
            {
                VKW_INIT_CHECK_BOOL(image_.init(
                    device,
                    VK_IMAGE_TYPE_2D,
                    depthStencilFormat,
                    VkExtent3D{w, h, 1},
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
            }

            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image
                = (externalImage_ != VK_NULL_HANDLE) ? externalImage_ : image_.getHandle();
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
            VKW_INIT_CHECK_VK(
                vkCreateImageView(device_->getHandle(), &createInfo, nullptr, &imageView_));

            extent_.width = w;
            extent_.height = h;

            initialized_ = true;
        }

        return true;
    }

  private:
    VkFormat format_{};

    VkAttachmentLoadOp loadPolicy_{VK_ATTACHMENT_LOAD_OP_DONT_CARE};
    VkAttachmentStoreOp storePolicy_{VK_ATTACHMENT_STORE_OP_DONT_CARE};
};
} // namespace vkw