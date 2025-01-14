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
#include "vkWrappers/wrappers/RenderPass.hpp"
#include "vkWrappers/wrappers/RenderTarget.hpp"
#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
class Framebuffer
{
  public:
    Framebuffer() {}
    Framebuffer(
        Device& device,
        RenderPass& renderpass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t layerCount = 1)
    {
        VKW_CHECK_BOOL_THROW(
            this->init(device, renderpass, w, h, layerCount), "Creating framebuffer");
    }

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&& cp) { *this = std::move(cp); }

    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer& operator=(Framebuffer&& cp)
    {
        this->clear();

        std::swap(device_, cp.device_);
        std::swap(renderpass_, cp.renderpass_);
        std::swap(framebuffer_, cp.framebuffer_);
        std::swap(imageViews_, cp.imageViews_);
        std::swap(extent_, cp.extent_);
        std::swap(cp.layerCount_, layerCount_);

        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    ~Framebuffer() { this->clear(); }

    auto getHandle() const { return framebuffer_; }
    auto getExtent() const { return extent_; }

    bool init(
        Device& device,
        RenderPass& renderpass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t layerCount = 1)
    {
        if(!initialized_)
        {
            device_ = &device;
            renderpass_ = &renderpass;

            extent_.width = w;
            extent_.height = h;
            layerCount_ = layerCount;
        }
        return true;
    }

    void clear()
    {
        VKW_DELETE_VK(Framebuffer, framebuffer_);

        device_ = nullptr;
        renderpass_ = nullptr;

        framebuffer_ = VK_NULL_HANDLE;

        extent_ = {};
        imageViews_.clear();
        layerCount_ = 0;
        initialized_ = false;
    }

    Framebuffer& addAttachment(const RenderTarget& attachment)
    {
        imageViews_.push_back(attachment.imageView());
        return *this;
    }

    void create()
    {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.pNext = nullptr;
        framebufferInfo.renderPass = renderpass_->getHandle();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews_.size());
        framebufferInfo.pAttachments = imageViews_.data();
        framebufferInfo.width = extent_.width;
        framebufferInfo.height = extent_.height;
        framebufferInfo.layers = layerCount_;
        VKW_CHECK_VK_THROW(
            vkCreateFramebuffer(device_->getHandle(), &framebufferInfo, nullptr, &framebuffer_),
            "Creating framebuffer");
    }

    bool isInitialized() const { return initialized_; }

  private:
    Device* device_{nullptr};
    RenderPass* renderpass_{nullptr};

    VkFramebuffer framebuffer_{VK_NULL_HANDLE};

    VkExtent2D extent_{};
    std::vector<VkImageView> imageViews_{};
    uint32_t layerCount_{0};

    bool initialized_{false};
};
} // namespace vkw