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
#include "vkWrappers/wrappers/RenderTarget.hpp"

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
class RenderPass
{
  public:
    RenderPass() {}
    RenderPass(Device& device);

    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&& rhs);

    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&& rhs);

    ~RenderPass() { this->clear(); }

    bool init(Device& device);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkRenderPass& getHandle() { return renderPass_; }
    const VkRenderPass& getHandle() const { return renderPass_; }

    bool useDepth() const { return depthStencilAttachments_.size() > 0; }

    RenderPass& addColorAttachment(
        const ColorRenderTarget& attachment,
        const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        const VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    RenderPass& addDepthStencilAttachment(
        const DepthStencilRenderTarget& attachment,
        const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        const VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    RenderPass& addColorAttachment(
        const VkFormat format,
        const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        const VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    RenderPass& addDepthStencilAttachment(
        const VkFormat format,
        const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        const VkImageLayout finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        const VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        const VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    RenderPass& addSubPass(
        const std::vector<uint32_t>& colorAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    RenderPass& addSubPass(
        const std::vector<uint32_t>& colorAttachments,
        const std::vector<uint32_t>& depthStencilAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    RenderPass& addSubPassWithResolve(
        const std::vector<uint32_t>& colorAttachments,
        const std::vector<uint32_t>& resolveAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    RenderPass& addSubPassWithResolve(
        const std::vector<uint32_t>& colorAttachments,
        const std::vector<uint32_t>& depthStencilAttachments,
        const std::vector<uint32_t>& resolveAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    RenderPass& addSubpassDependency(
        const uint32_t srcSubpass,
        const uint32_t dstSubpass,
        const VkPipelineStageFlags srcStageMask,
        const VkPipelineStageFlags dstStageMask,
        const VkAccessFlags srcAccessMask,
        const VkAccessFlags dstAccessFlags,
        const VkDependencyFlags flags = 0)
    {
        if(renderPass_ != VK_NULL_HANDLE)
        {
            throw std::runtime_error("Attempting to modify an already allocated RenderPass");
        }

        VkSubpassDependency dependency{};
        dependency.srcSubpass = srcSubpass;
        dependency.dstSubpass = dstSubpass;
        dependency.srcStageMask = srcStageMask;
        dependency.dstStageMask = dstStageMask;
        dependency.srcAccessMask = srcAccessMask;
        dependency.dstAccessMask = dstAccessFlags;
        dependency.dependencyFlags = flags;

        subpassDependencies_.emplace_back(dependency);
        return *this;
    }

    void create();

  private:
    Device* device_{nullptr};
    VkRenderPass renderPass_{VK_NULL_HANDLE};

    std::vector<VkAttachmentDescription> attachments_{};
    std::vector<VkAttachmentDescription> depthStencilAttachments_{};
    std::vector<VkAttachmentDescription> resolveAttachments_{};
    std::vector<VkSubpassDescription> subPasses_{};
    std::vector<VkSubpassDependency> subpassDependencies_{};

    std::vector<std::vector<VkAttachmentReference>> colorReferenceList_{};
    std::vector<std::vector<VkAttachmentReference>> depthStencilReferenceList_{};
    std::vector<std::vector<VkAttachmentReference>> resolveReferenceList_{};

    bool initialized_{false};
};
} // namespace vkw