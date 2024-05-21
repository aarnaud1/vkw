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
    // Update subpass info
    const bool useDepthStencil = !depthStencilReferenceList_.empty();
    for(size_t i = 0; i < colorReferenceList_.size(); ++i)
    {
        subPasses_[i].pColorAttachments = colorReferenceList_[i].data();
        if(useDepthStencil)
        {
            subPasses_[i].pDepthStencilAttachment = depthStencilReferenceList_[i].data();
        }
    }

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

RenderPass& RenderPass::addColorAttachment(
    const VkFormat format, const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription attachment{};
    attachment.format = format;
    attachment.samples = samples;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments_.emplace_back(attachment);
    return *this;
}

RenderPass& RenderPass::addDepthAttachment(
    const VkFormat format, const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription attachment{};
    attachment.format = format;
    attachment.samples = samples;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthStencilAttachments_.emplace_back(attachment);
    return *this;
}

RenderPass& RenderPass::addSubPass(
    const std::vector<uint32_t>& colorAttachments, const VkPipelineBindPoint bindPoint)
{
    if(renderPass_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempting to modify an already allocated RenderPass");
    }

    std::vector<VkAttachmentReference> colorAttachmentsList{};
    colorAttachmentsList.resize(colorAttachments.size());
    for(size_t i = 0; i < colorAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = colorAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentsList[i] = ref;
    }
    colorReferenceList_.emplace_back(std::move(colorAttachmentsList));

    VkSubpassDescription subPass{};
    subPass.flags = 0;
    subPass.pipelineBindPoint = bindPoint;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = nullptr;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subPass.pColorAttachments = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.pDepthStencilAttachment = nullptr;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = nullptr;

    subPasses_.emplace_back(subPass);
    return *this;
}
RenderPass& RenderPass::addSubPass(
    const std::vector<uint32_t>& colorAttachments,
    const std::vector<uint32_t>& depthStencilAttachments,
    const VkPipelineBindPoint bindPoint)
{
    if(renderPass_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Attempting to modify an already allocated RenderPass");
    }

    if(colorAttachments.size() != depthStencilAttachments.size())
    {
        throw std::runtime_error("Color and depth attachment counts must be equal");
    }

    std::vector<VkAttachmentReference> colorAttachmentsList{};
    colorAttachmentsList.resize(colorAttachments.size());
    for(size_t i = 0; i < colorAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = colorAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentsList[i] = ref;
    }
    colorReferenceList_.emplace_back(std::move(colorAttachmentsList));

    std::vector<VkAttachmentReference> depthStencilAttachmentsList{};
    depthStencilAttachmentsList.resize(depthStencilAttachments.size());
    for(size_t i = 0; i < depthStencilAttachments.size(); ++i)
    {
        VkAttachmentReference ref{};
        ref.attachment = depthStencilAttachments[i];
        ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthStencilAttachmentsList[i] = ref;
    }
    depthStencilReferenceList_.emplace_back(std::move(depthStencilAttachmentsList));

    VkSubpassDescription subPass{};
    subPass.flags = 0;
    subPass.pipelineBindPoint = bindPoint;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = nullptr;
    subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subPass.pColorAttachments = nullptr;
    subPass.pResolveAttachments = nullptr;
    subPass.pDepthStencilAttachment = nullptr;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = nullptr;

    subPasses_.emplace_back(subPass);
    return *this;
}
} // namespace vk