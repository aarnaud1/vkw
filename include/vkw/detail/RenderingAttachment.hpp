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
#include "vkw/detail/ImageView.hpp"

namespace vkw
{
class RenderingAttachment
{
  public:
    RenderingAttachment() = default;
    RenderingAttachment(
        const ImageView& imageView, const VkImageLayout imageLayout, const VkClearValue clearValue = {},
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE)
        : attachment_{imageView.getHandle()}
        , imageLayout_{imageLayout}
        , clearValue_{clearValue}
        , loadOp_{loadOp}
        , storeOp_{storeOp}
    {}
    RenderingAttachment(
        const ImageView& imageView, const VkImageLayout imageLayout, const ImageView& resolveImageView,
        const VkImageLayout resolveImageLayout, const VkResolveModeFlagBits resolveMode,
        const VkClearValue clearValue = {}, const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE)
        : attachment_{imageView.getHandle()}
        , imageLayout_{imageLayout}
        , resolveAttachment_{resolveImageView.getHandle()}
        , resolveImageLayout_{resolveImageLayout}
        , resolveMode_{resolveMode}
        , clearValue_{clearValue}
        , loadOp_{loadOp}
        , storeOp_{storeOp}
    {}

    RenderingAttachment(const RenderingAttachment&) = default;
    RenderingAttachment(RenderingAttachment&&) = default;

    RenderingAttachment& operator=(const RenderingAttachment&) = default;
    RenderingAttachment& operator=(RenderingAttachment&&) = default;

    ~RenderingAttachment() = default;

    auto imageView() const { return attachment_; }
    auto resolveImageView() const { return resolveAttachment_; }

    auto loadOp() const { return loadOp_; }
    auto storeOp() const { return storeOp_; }

  private:
    friend class CommandBuffer;

    VkImageView attachment_{VK_NULL_HANDLE};
    VkImageLayout imageLayout_{VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageView resolveAttachment_{VK_NULL_HANDLE};
    VkImageLayout resolveImageLayout_{VK_IMAGE_LAYOUT_UNDEFINED};
    VkResolveModeFlagBits resolveMode_{VK_RESOLVE_MODE_NONE};

    VkClearValue clearValue_{};
    VkAttachmentLoadOp loadOp_{};
    VkAttachmentStoreOp storeOp_{};
};
} // namespace vkw