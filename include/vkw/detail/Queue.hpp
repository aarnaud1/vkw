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
#include "vkw/detail/utils.hpp"

#include <span>

namespace vkw
{
enum QueueUsageBits
{
    Graphics = 0x01,
    Compute = 0x02,
    Transfer = 0x04,
    SparseBinding = 0x08,
    Protected = 0x10,
    VideoDecode = 0x20,
    VideoEncode = 0x40
};
typedef uint32_t QueueUsageFlags;

///@note: We need to use forward declarations here because the Queue class is exposed before these classes are
///       defined.
class CommandBuffer;
class Fence;
class Semaphore;
class TimelineSemaphore;
class Swapchain;

class Queue
{
  public:
    Queue() = default;
    Queue(const VolkDeviceTable& vkFuncs) : vk{&vkFuncs} {}

    Queue(const Queue& cp) = default;
    Queue(Queue&&) = default;

    Queue& operator=(const Queue& cp) = default;
    Queue& operator=(Queue&&) = default;

    ~Queue() {}

    bool supportsPresent(const VkSurfaceKHR surface) const;

    VkQueueFlags flags() const { return flags_; }
    uint32_t queueFamilyIndex() const { return queueFamilyIndex_; }
    uint32_t queueIndex() const { return queueIndex_; }

    VkQueue getHandle() const { return queue_; }

    VkResult submit(const CommandBuffer& cmdBuffer, const Fence& fence) const;

    VkResult submit(
        const CommandBuffer& cmdBuffer,
        const std::initializer_list<std::reference_wrapper<Semaphore>>& waitSemaphores,
        const std::span<VkPipelineStageFlags>& waitFlags,
        const std::initializer_list<std::reference_wrapper<Semaphore>>& signalSemaphores) const;
    VkResult submit(
        const CommandBuffer& cmdBuffer,
        const std::initializer_list<std::reference_wrapper<Semaphore>>& waitSemaphores,
        const std::span<VkPipelineStageFlags>& waitFlags,
        const std::initializer_list<std::reference_wrapper<Semaphore>>& signalSemaphores,
        const Fence& fence) const;
    VkResult submit(
        const VkCommandBuffer cmdBuffer, const std::span<VkSemaphore>& waitSemaphores,
        const std::span<VkPipelineStageFlags>& waitFlags, const std::span<VkSemaphore>& signalSemaphores,
        const VkFence fence = VK_NULL_HANDLE) const;

    VkResult submit(
        const CommandBuffer& cmdBuffer, const TimelineSemaphore& semaphore,
        const VkPipelineStageFlags waitFlags, const uint64_t waitValue, const uint64_t signalValue) const;
    VkResult submit(
        const CommandBuffer& cmdBuffer, const TimelineSemaphore& semaphore,
        const VkPipelineStageFlags waitFlags, const uint64_t waitValue, const uint64_t signalValue,
        const Fence& fence) const;
    VkResult submit(
        const VkCommandBuffer cmdBuffer, const VkSemaphore& semaphore, const VkPipelineStageFlags waitFlags,
        const uint64_t waitValue, const uint64_t signalValue, const VkFence fence = VK_NULL_HANDLE) const;

    VkResult submit(
        const CommandBuffer& cmdBuffer,
        const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& waitSemaphores,
        const std::span<VkPipelineStageFlags>& waitFlags, const std::span<uint64_t>& waitValues,
        const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& signalSemaphores,
        const std::span<uint64_t>& signalValues) const;
    VkResult submit(
        const CommandBuffer& cmdBuffer,
        const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& waitSemaphores,
        const std::span<VkPipelineStageFlags>& waitFlags, const std::span<uint64_t>& waitValues,
        const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& signalSemaphores,
        const std::span<uint64_t>& signalValues, const Fence& fence) const;
    VkResult submit(
        const VkCommandBuffer cmdBuffer, const std::span<VkSemaphore>& waitSemaphores,
        const std::span<VkPipelineStageFlags>& waitFlags, const std::span<uint64_t>& waitValues,
        const std::span<VkSemaphore>& signalSemaphores, const std::span<uint64_t>& signalValues,
        const VkFence = VK_NULL_HANDLE) const;

    VkResult present(
        const Swapchain& swapchain, const Semaphore& waitSemaphore, const uint32_t imageIndex) const;
    VkResult present(
        const VkSwapchainKHR swapchain, const VkSemaphore waitSemaphore, const uint32_t imageIndex) const;

    VkResult present(
        const Swapchain& swapchain,
        const std::initializer_list<std::reference_wrapper<Semaphore>>& waitSemaphores,
        const uint32_t imageIndex) const;
    VkResult present(
        const VkSwapchainKHR& swapchain, const std::span<VkSemaphore>& waitSemaphores,
        const uint32_t imageIndex) const;

    VkResult waitIdle() const { return vk->vkQueueWaitIdle(queue_); }

  private:
    friend class Device;
    const VolkDeviceTable* vk;

    QueueUsageFlags flags_{0};

    uint32_t queueFamilyIndex_{0};
    uint32_t queueIndex_{0};

    VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
    VkQueue queue_{VK_NULL_HANDLE};
};
} // namespace vkw
