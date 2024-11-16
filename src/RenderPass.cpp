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

#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
RenderPass::RenderPass(Device& device)
{
    VKW_CHECK_BOOL_THROW(this->init(device), "Creating render pass");
}

RenderPass::RenderPass(RenderPass&& cp) { *this = std::move(cp); }

RenderPass& RenderPass::operator=(RenderPass&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(renderPass_, cp.renderPass_);

    std::swap(attachments_, cp.attachments_);
    std::swap(depthStencilAttachments_, cp.depthStencilAttachments_);
    std::swap(subPasses_, cp.subPasses_);
    std::swap(subpassDependencies_, cp.subpassDependencies_);

    std::swap(colorReferenceList_, cp.colorReferenceList_);
    std::swap(depthStencilReferenceList_, cp.depthStencilReferenceList_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

bool RenderPass::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;
        initialized_ = true;
    }

    return true;
}

void RenderPass::clear()
{
    VKW_DELETE_VK(RenderPass, renderPass_);

    attachments_.clear();
    depthStencilAttachments_.clear();
    subPasses_.clear();
    subpassDependencies_.clear();

    colorReferenceList_.clear();
    depthStencilReferenceList_.clear();

    device_ = nullptr;
    initialized_ = false;
}

void RenderPass::create()
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

    std::vector<VkAttachmentDescription> attachmentList;
    attachmentList.insert(attachmentList.end(), attachments_.begin(), attachments_.end());
    if(useDepthStencil)
    {
        attachmentList.insert(
            attachmentList.end(), depthStencilAttachments_.begin(), depthStencilAttachments_.end());
    }

    VkRenderPassCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = static_cast<uint32_t>(attachmentList.size());
    createInfo.pAttachments = attachmentList.data();
    createInfo.subpassCount = static_cast<uint32_t>(subPasses_.size());
    createInfo.pSubpasses = subPasses_.data();
    createInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies_.size());
    createInfo.pDependencies = subpassDependencies_.data();
    VKW_CHECK_VK_THROW(
        vkCreateRenderPass(device_->getHandle(), &createInfo, nullptr, &renderPass_),
        "Creating render pass");
}

RenderPass& RenderPass::addColorAttachment(
    const ColorRenderTarget& attachment,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = attachment.format();
    descr.samples = samples;
    descr.loadOp = attachment.getLoadPolicy();
    descr.storeOp = attachment.getStorePolicy();
    descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    attachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addDepthStencilAttachment(
    const DepthRenderTarget& attachment,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = attachment.format();
    descr.samples = samples;
    descr.loadOp = attachment.getDepthLoadPolicy();
    descr.storeOp = attachment.getDepthStorePolicy();
    descr.stencilLoadOp = attachment.getStencilLoadPolicy();
    descr.stencilStoreOp = attachment.getStencilStorePolicy();
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    depthStencilAttachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addColorAttachment(
    const VkFormat format,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = format;
    descr.samples = samples;
    descr.loadOp = loadOp;
    descr.storeOp = storeOp;
    descr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    attachments_.emplace_back(descr);
    return *this;
}

RenderPass& RenderPass::addDepthStencilAttachment(
    const VkFormat format,
    const VkImageLayout initialLayout,
    const VkImageLayout finalLayout,
    const VkAttachmentLoadOp loadOp,
    const VkAttachmentStoreOp storeOp,
    const VkAttachmentLoadOp stencilLoadOp,
    const VkAttachmentStoreOp stencilStoreOp,
    const VkSampleCountFlagBits samples)
{
    VkAttachmentDescription descr{};
    descr.format = format;
    descr.samples = samples;
    descr.loadOp = loadOp;
    descr.storeOp = storeOp;
    descr.stencilLoadOp = stencilLoadOp;
    descr.stencilStoreOp = stencilStoreOp;
    descr.initialLayout = initialLayout;
    descr.finalLayout = finalLayout;

    depthStencilAttachments_.emplace_back(descr);
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
} // namespace vkw