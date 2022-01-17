/*
 * Copyright (C) 2022 Adrien ARNAUD
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

#include <vulkan/vulkan.h>

#include "utils.hpp"
#include "QueueFamilies.hpp"

// Weird design, but encapsulate Queue operation from user side
namespace vk
{
template <QueueFamilyType type>
class Queue
{
public:
  Queue() : queue_(VK_NULL_HANDLE) {}

  Queue &submit(
      std::vector<VkCommandBuffer> &cmdBuffers, VkFence fence = VK_NULL_HANDLE)
  {
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        cmdBuffers.size(),
        cmdBuffers.data(),
        0,
        nullptr};
    vkQueueSubmit(queue_, 1, &submitInfo, fence);
    return *this;
  }

  Queue &submit(VkCommandBuffer cmdBuffer, VkFence fence = VK_NULL_HANDLE)
  {
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        1,
        &cmdBuffer,
        0,
        nullptr};
    vkQueueSubmit(queue_, 1, &submitInfo, fence);
    return *this;
  }

  Queue &waitIdle()
  {
    vkQueueWaitIdle(queue_);
    return *this;
  }

  VkQueue &getHandle() { return queue_; }

private:
  VkQueue queue_;
};
} // namespace vk
