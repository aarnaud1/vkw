/*
 * Copyright (c) 2026 Adrien ARNAUD
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
#include "vkw/detail/ImageView.hpp"
#include "vkw/detail/RenderPass.hpp"
#include "vkw/detail/utils.hpp"

namespace vkw
{
class Framebuffer
{
  public:
    Framebuffer() {}
    Framebuffer(
        const Device& device, const RenderPass& renderpass, const uint32_t w, const uint32_t h,
        const uint32_t layerCount = 1)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, renderpass, w, h, layerCount), "Creating framebuffer");
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
        const Device& device, const RenderPass& renderpass, const uint32_t w, const uint32_t h,
        const uint32_t layerCount = 1)
    {
        VKW_ASSERT(this->initialized() == false);

        device_ = &device;
        renderpass_ = &renderpass;

        extent_.width = w;
        extent_.height = h;
        layerCount_ = layerCount;

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

    Framebuffer& addAttachment(const ImageView& imageView)
    {
        imageViews_.push_back(imageView.getHandle());
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
        VKW_CHECK_VK_FAIL(
            device_->vk().vkCreateFramebuffer(device_->getHandle(), &framebufferInfo, nullptr, &framebuffer_),
            "Creating framebuffer");
    }

    bool initialized() const { return initialized_; }

  private:
    const Device* device_{nullptr};
    const RenderPass* renderpass_{nullptr};

    VkFramebuffer framebuffer_{VK_NULL_HANDLE};

    VkExtent2D extent_{};
    std::vector<VkImageView> imageViews_{};
    uint32_t layerCount_{0};

    bool initialized_{false};
};
} // namespace vkw
