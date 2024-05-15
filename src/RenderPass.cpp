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

#include "vkWrappers/wrappers/RenderPass.hpp"

namespace vk
{
RenderPass& RenderPass::create()
{
    VkRenderPassCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = static_cast<uint32_t>(attachments_.size());
    createInfo.pAttachments = attachments_.data();
    createInfo.subpassCount = static_cast<uint32_t>(subPasses_.size());
    createInfo.pSubpasses = subPasses_.data();
    createInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies_.size());
    createInfo.pDependencies = subpassDependencies_.data();
    CHECK_VK(
        vkCreateRenderPass(device_->getHandle(), &createInfo, nullptr, &renderPass_),
        "Creating render pass");

    return *this;
}

RenderPass& RenderPass::release()
{
    vkDestroyRenderPass(device_->getHandle(), renderPass_, nullptr);
    renderPass_ = VK_NULL_HANDLE;
    return *this;
}
} // namespace vk