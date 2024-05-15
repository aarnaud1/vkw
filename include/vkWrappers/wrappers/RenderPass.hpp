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

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "vkWrappers/wrappers/Device.hpp"

namespace vk
{
class RenderPass
{
  public:
    RenderPass(Device &device) : device_{&device} {}

    RenderPass(const RenderPass &) = delete;
    RenderPass(RenderPass &&) = delete;

    RenderPass &operator=(const RenderPass &) = delete;
    RenderPass &operator=(RenderPass &&) = delete;

    ~RenderPass() { release(); }

    VkRenderPass getHandle() const { return renderPass_; }

    RenderPass &addAttachment(const VkAttachmentDescription &attachment)
    {
        if(renderPass_ != VK_NULL_HANDLE)
        {
            throw std::runtime_error("Attempting to modify an already allocated RenderPass");
        }
        attachments_.emplace_back(attachment);
        return *this;
    }

    RenderPass &addSubPass(const VkSubpassDescription &subPass)
    {
        if(renderPass_ != VK_NULL_HANDLE)
        {
            throw std::runtime_error("Attempting to modify an already allocated RenderPass");
        }
        subPasses_.emplace_back(subPass);
        return *this;
    }

    RenderPass &addSubpassDependency(const VkSubpassDependency &dependency)
    {
        if(renderPass_ != VK_NULL_HANDLE)
        {
            throw std::runtime_error("Attempting to modify an already allocated RenderPass");
        }
        subpassDependencies_.emplace_back(dependency);
        return *this;
    }

    RenderPass &create();
    RenderPass &release();

  private:
    Device *device_;
    VkRenderPass renderPass_{VK_NULL_HANDLE};

    std::vector<VkAttachmentDescription> attachments_{};
    std::vector<VkSubpassDescription> subPasses_{};
    std::vector<VkSubpassDependency> subpassDependencies_{};
};
} // namespace vk