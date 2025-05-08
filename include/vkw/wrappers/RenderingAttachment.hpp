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

#include "vkw/wrappers/Common.hpp"
#include "vkw/wrappers/Device.hpp"
#include "vkw/wrappers/ImageView.hpp"

namespace vkw
{
class RenderingAttachment
{
  public:
    RenderingAttachment() = default;
    RenderingAttachment(
        const ImageView& imageView,
        const VkImageLayout imageLayout,
        const VkClearValue clearValue = {},
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE)
        : attachment_{imageView.getHandle()}
        , imageLayout_{imageLayout}
        , clearValue_{clearValue}
        , loadOp_{loadOp}
        , storeOp_{storeOp}
    {}
    RenderingAttachment(
        const ImageView& imageView,
        const VkImageLayout imageLayout,
        const ImageView& resolveImageView,
        const VkImageLayout resolveImageLayout,
        const VkResolveModeFlagBits resolveMode,
        const VkClearValue clearValue = {},
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
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