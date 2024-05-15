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

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include "vkWrappers/wrappers/utils.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/QueueFamilies.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/DescriptorSetLayout.hpp"

namespace vk
{
class PipelineLayout
{
  public:
    PipelineLayout(Device &device, size_t numSets);

    ~PipelineLayout();

    void create();

    uint32_t addPushConstantRange(VkShaderStageFlags stages, uint32_t size)
    {
        VkPushConstantRange range = {};
        range.stageFlags = stages;
        range.offset = offset_;
        range.size = size;
        pushConstantRanges_.push_back(range);

        uint32_t ret = offset_;
        // Offset must be a multiple of four
        offset_ += size + (4 - size % 4);

        return ret;
    }

    size_t numSets() const { return setLayouts_.size(); }

    DescriptorSetLayout &getDescriptorSetlayoutInfo(const size_t i) { return setLayoutInfo_[i]; }

    std::vector<VkDescriptorSetLayout> &getDescriptorSetLayouts() { return setLayouts_; }

    VkPipelineLayout &getHandle() { return layout_; }

  private:
    Device &device_;
    std::vector<DescriptorSetLayout> setLayoutInfo_{};
    std::vector<VkDescriptorSetLayout> setLayouts_{};
    VkPipelineLayout layout_{VK_NULL_HANDLE};

    uint32_t offset_ = 0;
    std::vector<VkPushConstantRange> pushConstantRanges_{};

    void createDescriptorSetLayouts();
};
} // namespace vk
