/*
 * Copyright (c) 2025 Adrien ARNAUD
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

#include <vector>

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

class Queue
{
    ///@note : because Device is not visible from this class, methods must use templates to
    /// designate other vkw objects here.
    ///@note : we could create "base" class objects with the name and a valid getHandle() method, if
    /// compilation is too long.
  public:
    Queue() = default;
    Queue(const VolkDeviceTable& vkFuncs) : vk{&vkFuncs} {}

    Queue(const Queue& cp) = default;
    Queue(Queue&&) = default;

    Queue& operator=(const Queue& cp) = default;
    Queue& operator=(Queue&&) = default;

    ~Queue() {}

    bool supportsPresent(const VkSurfaceKHR surface) const
    {
        if(surface != VK_NULL_HANDLE)
        {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice_, queueFamilyIndex_, surface, &presentSupport);
            return (presentSupport == VK_TRUE) ? true : false;
        }
        return false;
    }

    VkQueueFlags flags() const { return flags_; }
    uint32_t queueFamilyIndex() const { return queueFamilyIndex_; }
    uint32_t queueIndex() const { return queueIndex_; }

    VkQueue getHandle() const { return queue_; }

    // ---------------------------------------------------------------------------------------------

    template <typename CommandBuffer, typename Fence>
    VkResult submit(const CommandBuffer& cmdBuffer, const Fence& fence) const
    {
        const auto handle = cmdBuffer.getHandle();
        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &(handle), 0, nullptr};
        return vk->vkQueueSubmit(queue_, 1, &submitInfo, fence.getHandle());
    }

    template <typename CommandBuffer, typename Semaphore>
    VkResult submit(
        const CommandBuffer& cmdBuffer, const std::vector<Semaphore*>& waitSemaphores,
        const std::vector<VkPipelineStageFlags>& waitFlags,
        const std::vector<Semaphore*>& signalSemaphores) const
    {
        const auto handle = cmdBuffer.getHandle();

        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        std::vector<VkSemaphore> signalSemaphoreValues;
        signalSemaphoreValues.reserve(signalSemaphores.size());
        for(size_t i = 0; i < signalSemaphores.size(); ++i)
        {
            signalSemaphoreValues.emplace_back(signalSemaphores[i]->getHandle());
        }

        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               static_cast<uint32_t>(waitSemaphores.size()),
               waitSemaphoreValues.data(),
               waitFlags.data(),
               1,
               &(handle),
               static_cast<uint32_t>(signalSemaphores.size()),
               signalSemaphoreValues.data()};
        return vk->vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);
    }

    template <typename CommandBuffer, typename TimelineSemaphore>
    VkResult submit(
        const CommandBuffer& cmdBuffer, const TimelineSemaphore& semaphore,
        const VkPipelineStageFlags waitFlags, const uint64_t waitValue, const uint64_t signalValue) const
    {
        const auto handle = cmdBuffer.getHandle();
        const auto semHandle = semaphore.getHandle();

        VkTimelineSemaphoreSubmitInfo semaphoreSubmitInfo = {};
        semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.pNext = nullptr;
        semaphoreSubmitInfo.waitSemaphoreValueCount = 1;
        semaphoreSubmitInfo.pWaitSemaphoreValues = &waitValue;
        semaphoreSubmitInfo.signalSemaphoreValueCount = 1;
        semaphoreSubmitInfo.pSignalSemaphoreValues = &signalValue;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = &semaphoreSubmitInfo;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitDstStageMask = &waitFlags;
        submitInfo.pWaitSemaphores = &(semHandle);
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &(semHandle);
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(handle);
        return vk->vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);
    }

    template <typename CommandBuffer, typename TimelineSemaphore>
    VkResult submit(
        const CommandBuffer& cmdBuffer, const std::vector<TimelineSemaphore*>& waitSemaphores,
        const std::vector<VkPipelineStageFlags>& waitFlags, const std::vector<uint64_t>& waitValues,
        const std::vector<TimelineSemaphore*>& signalSemaphores,
        const std::vector<uint64_t>& signalValues) const
    {
        const auto handle = cmdBuffer.getHandle();

        VkTimelineSemaphoreSubmitInfo semaphoreSubmitInfo = {};
        semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        semaphoreSubmitInfo.pNext = nullptr;
        semaphoreSubmitInfo.waitSemaphoreValueCount = static_cast<uint32_t>(waitSemaphores.size());
        semaphoreSubmitInfo.pWaitSemaphoreValues = waitValues.data();
        semaphoreSubmitInfo.signalSemaphoreValueCount = static_cast<uint32_t>(signalSemaphores.size());
        semaphoreSubmitInfo.pSignalSemaphoreValues = signalValues.data();

        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        std::vector<VkSemaphore> signalSemaphoreValues;
        signalSemaphoreValues.reserve(signalSemaphores.size());
        for(size_t i = 0; i < signalSemaphores.size(); ++i)
        {
            signalSemaphoreValues.emplace_back(signalSemaphores[i]->getHandle());
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = &semaphoreSubmitInfo;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        submitInfo.pWaitDstStageMask = waitFlags.data();
        submitInfo.pWaitSemaphores = waitSemaphoreValues.data();
        submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
        submitInfo.pSignalSemaphores = signalSemaphoreValues.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(handle);
        return vk->vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);
    }

    template <typename CommandBuffer, typename Semaphore, typename Fence>
    VkResult submit(
        const CommandBuffer& cmdBuffer, const std::vector<Semaphore*>& waitSemaphores,
        const std::vector<VkPipelineStageFlags>& waitFlags, const std::vector<Semaphore*>& signalSemaphores,
        const Fence& fence) const
    {
        const auto handle = cmdBuffer.getHandle();

        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        std::vector<VkSemaphore> signalSemaphoreValues;
        signalSemaphoreValues.reserve(signalSemaphores.size());
        for(size_t i = 0; i < signalSemaphores.size(); ++i)
        {
            signalSemaphoreValues.emplace_back(signalSemaphores[i]->getHandle());
        }

        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               static_cast<uint32_t>(waitSemaphores.size()),
               waitSemaphoreValues.data(),
               waitFlags.data(),
               1,
               &(handle),
               static_cast<uint32_t>(signalSemaphores.size()),
               signalSemaphoreValues.data()};
        return vk->vkQueueSubmit(queue_, 1, &submitInfo, fence.getHandle());
    }

    // ---------------------------------------------------------------------------------------------

    template <typename Swapchain, typename Semaphore>
    VkResult present(
        const Swapchain& swapchain, const Semaphore& waitSemaphore, const uint32_t imageIndex) const
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore.getHandle();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.getHandle();
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        return vk->vkQueuePresentKHR(queue_, &presentInfo);
    }

    template <typename Swapchain, typename Semaphore>
    VkResult present(
        const Swapchain& swapchain, const std::vector<Semaphore*>& waitSemaphores,
        const uint32_t imageIndex) const
    {
        std::vector<VkSemaphore> waitSemaphoreValues;
        waitSemaphoreValues.reserve(waitSemaphores.size());
        for(size_t i = 0; i < waitSemaphores.size(); ++i)
        {
            waitSemaphoreValues.emplace_back(waitSemaphores[i]->getHandle());
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphoreValues.size());
        presentInfo.pWaitSemaphores = waitSemaphoreValues.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.getHandle();
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        return vk->vkQueuePresentKHR(queue_, &presentInfo);
    }

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
