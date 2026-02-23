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

#include <stdexcept>
#include <vector>

namespace vkw
{
class RenderPass
{
  public:
    RenderPass() {}
    RenderPass(const Device& device);

    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&& rhs);

    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&& rhs);

    ~RenderPass() { this->clear(); }

    bool init(const Device& device);

    void clear();

    bool initialized() const { return initialized_; }

    VkRenderPass& getHandle() { return renderPass_; }
    const VkRenderPass& getHandle() const { return renderPass_; }

    bool useDepth() const { return depthStencilAttachments_.size() > 0; }

    RenderPass& addColorAttachment(
        const VkFormat format, const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        const VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        const VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        const VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        const VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    RenderPass& addDepthStencilAttachment(
        const VkFormat format, const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
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
        const std::vector<uint32_t>& colorAttachments, const std::vector<uint32_t>& depthStencilAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    RenderPass& addSubPassWithResolve(
        const std::vector<uint32_t>& colorAttachments, const std::vector<uint32_t>& resolveAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    RenderPass& addSubPassWithResolve(
        const std::vector<uint32_t>& colorAttachments, const std::vector<uint32_t>& depthStencilAttachments,
        const std::vector<uint32_t>& resolveAttachments,
        const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    RenderPass& addSubpassDependency(
        const uint32_t srcSubpass, const uint32_t dstSubpass, const VkPipelineStageFlags srcStageMask,
        const VkPipelineStageFlags dstStageMask, const VkAccessFlags srcAccessMask,
        const VkAccessFlags dstAccessFlags, const VkDependencyFlags flags = 0)
    {
        VKW_ASSERT(this->initialized());

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
    const Device* device_{nullptr};
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
