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

#include "vkw/detail/Queue.hpp"

#include "vkw/detail/CommandBuffer.hpp"
#include "vkw/detail/Swapchain.hpp"
#include "vkw/detail/Synchronization.hpp"

namespace vkw
{
bool Queue::supportsPresent(const VkSurfaceKHR surface) const
{
    if(surface != VK_NULL_HANDLE)
    {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, queueFamilyIndex_, surface, &presentSupport);
        return (presentSupport == VK_TRUE) ? true : false;
    }
    return false;
}

// -----------------------------------------------------------------------------------------------------------

VkResult Queue::submit(const CommandBuffer& cmdBuffer, const Fence& fence) const
{
    const auto handle = cmdBuffer.getHandle();
    VkSubmitInfo submitInfo
        = {VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &(handle), 0, nullptr};
    return vk->vkQueueSubmit(queue_, 1, &submitInfo, fence.getHandle());
}

// -----------------------------------------------------------------------------------------------------------

VkResult Queue::submit(
    const CommandBuffer& cmdBuffer,
    const std::initializer_list<std::reference_wrapper<Semaphore>>& waitSemaphores,
    const std::span<VkPipelineStageFlags>& waitFlags,
    const std::initializer_list<std::reference_wrapper<Semaphore>>& signalSemaphores) const
{
    std::vector<VkSemaphore> waitSemaphoreValues;
    waitSemaphoreValues.reserve(waitSemaphores.size());
    for(const auto& waitSemaphore : waitSemaphores)
    {
        waitSemaphoreValues.emplace_back(waitSemaphore.get().getHandle());
    }

    std::vector<VkSemaphore> signalSemaphoreValues;
    signalSemaphoreValues.reserve(signalSemaphores.size());
    for(const auto& signalSemaphore : signalSemaphores)
    {
        signalSemaphoreValues.emplace_back(signalSemaphore.get().getHandle());
    }

    return submit(
        cmdBuffer.getHandle(), waitSemaphoreValues, waitFlags, signalSemaphoreValues, VK_NULL_HANDLE);
}

VkResult Queue::submit(
    const CommandBuffer& cmdBuffer,
    const std::initializer_list<std::reference_wrapper<Semaphore>>& waitSemaphores,
    const std::span<VkPipelineStageFlags>& waitFlags,
    const std::initializer_list<std::reference_wrapper<Semaphore>>& signalSemaphores,
    const Fence& fence) const
{
    std::vector<VkSemaphore> waitSemaphoreValues;
    waitSemaphoreValues.reserve(waitSemaphores.size());
    for(const auto& waitSemaphore : waitSemaphores)
    {
        waitSemaphoreValues.emplace_back(waitSemaphore.get().getHandle());
    }

    std::vector<VkSemaphore> signalSemaphoreValues;
    signalSemaphoreValues.reserve(signalSemaphores.size());
    for(const auto& signalSemaphore : signalSemaphores)
    {
        signalSemaphoreValues.emplace_back(signalSemaphore.get().getHandle());
    }

    return submit(
        cmdBuffer.getHandle(), waitSemaphoreValues, waitFlags, signalSemaphoreValues, fence.getHandle());
}

VkResult Queue::submit(
    const VkCommandBuffer cmdBuffer, const std::span<VkSemaphore>& waitSemaphores,
    const std::span<VkPipelineStageFlags>& waitFlags, const std::span<VkSemaphore>& signalSemaphores,
    const VkFence fence) const
{
    VKW_ASSERT(waitFlags.size() == waitSemaphores.size());

    VkSubmitInfo submitInfo
        = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
           nullptr,
           static_cast<uint32_t>(waitSemaphores.size()),
           waitSemaphores.data(),
           waitFlags.data(),
           1,
           &(cmdBuffer),
           static_cast<uint32_t>(signalSemaphores.size()),
           signalSemaphores.data()};
    return vk->vkQueueSubmit(queue_, 1, &submitInfo, fence);
}

// -----------------------------------------------------------------------------------------------------------
VkResult Queue::submit(
    const CommandBuffer& cmdBuffer, const TimelineSemaphore& semaphore, const VkPipelineStageFlags waitFlags,
    const uint64_t waitValue, const uint64_t signalValue) const
{
    return submit(
        cmdBuffer.getHandle(), semaphore.getHandle(), waitFlags, waitValue, signalValue, VK_NULL_HANDLE);
}

VkResult Queue::submit(
    const CommandBuffer& cmdBuffer, const TimelineSemaphore& semaphore, const VkPipelineStageFlags waitFlags,
    const uint64_t waitValue, const uint64_t signalValue, const Fence& fence) const
{
    return submit(
        cmdBuffer.getHandle(), semaphore.getHandle(), waitFlags, waitValue, signalValue, fence.getHandle());
}

VkResult Queue::submit(
    const VkCommandBuffer cmdBuffer, const VkSemaphore& semaphore, const VkPipelineStageFlags waitFlags,
    const uint64_t waitValue, const uint64_t signalValue, const VkFence fence) const
{
    const auto semHandle = semaphore;

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
    submitInfo.pCommandBuffers = &(cmdBuffer);
    return vk->vkQueueSubmit(queue_, 1, &submitInfo, fence);
}

// -----------------------------------------------------------------------------------------------------------

VkResult Queue::submit(
    const CommandBuffer& cmdBuffer,
    const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& waitSemaphores,
    const std::span<VkPipelineStageFlags>& waitFlags, const std::span<uint64_t>& waitValues,
    const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& signalSemaphores,
    const std::span<uint64_t>& signalValues) const
{
    std::vector<VkSemaphore> waitSemaphoreValues;
    waitSemaphoreValues.reserve(waitSemaphores.size());
    for(const auto& waitSemaphore : waitSemaphores)
    {
        waitSemaphoreValues.emplace_back(waitSemaphore.get().getHandle());
    }

    std::vector<VkSemaphore> signalSemaphoreValues;
    signalSemaphoreValues.reserve(signalSemaphores.size());
    for(const auto& signalSemaphore : signalSemaphores)
    {
        signalSemaphoreValues.emplace_back(signalSemaphore.get().getHandle());
    }

    return submit(
        cmdBuffer.getHandle(), waitSemaphoreValues, waitFlags, waitValues, signalSemaphoreValues,
        signalValues, VK_NULL_HANDLE);
}

VkResult Queue::submit(
    const CommandBuffer& cmdBuffer,
    const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& waitSemaphores,
    const std::span<VkPipelineStageFlags>& waitFlags, const std::span<uint64_t>& waitValues,
    const std::initializer_list<std::reference_wrapper<TimelineSemaphore>>& signalSemaphores,
    const std::span<uint64_t>& signalValues, const Fence& fence) const
{
    std::vector<VkSemaphore> waitSemaphoreValues;
    waitSemaphoreValues.reserve(waitSemaphores.size());
    for(const auto& waitSemaphore : waitSemaphores)
    {
        waitSemaphoreValues.emplace_back(waitSemaphore.get().getHandle());
    }

    std::vector<VkSemaphore> signalSemaphoreValues;
    signalSemaphoreValues.reserve(signalSemaphores.size());
    for(const auto& signalSemaphore : signalSemaphores)
    {
        signalSemaphoreValues.emplace_back(signalSemaphore.get().getHandle());
    }

    return submit(
        cmdBuffer.getHandle(), waitSemaphoreValues, waitFlags, waitValues, signalSemaphoreValues,
        signalValues, fence.getHandle());
}

VkResult Queue::submit(
    const VkCommandBuffer cmdBuffer, const std::span<VkSemaphore>& waitSemaphores,
    const std::span<VkPipelineStageFlags>& waitFlags, const std::span<uint64_t>& waitValues,
    const std::span<VkSemaphore>& signalSemaphores, const std::span<uint64_t>& signalValues,
    const VkFence fence) const
{
    VKW_ASSERT(waitFlags.size() == waitValues.size());

    VkTimelineSemaphoreSubmitInfo semaphoreSubmitInfo = {};
    semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    semaphoreSubmitInfo.pNext = nullptr;
    semaphoreSubmitInfo.waitSemaphoreValueCount = static_cast<uint32_t>(waitSemaphores.size());
    semaphoreSubmitInfo.pWaitSemaphoreValues = waitValues.data();
    semaphoreSubmitInfo.signalSemaphoreValueCount = static_cast<uint32_t>(signalSemaphores.size());
    semaphoreSubmitInfo.pSignalSemaphoreValues = signalValues.data();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = &semaphoreSubmitInfo;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitDstStageMask = waitFlags.data();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(cmdBuffer);
    return vk->vkQueueSubmit(queue_, 1, &submitInfo, fence);
}

// -----------------------------------------------------------------------------------------------------------

VkResult Queue::present(
    const Swapchain& swapchain, const Semaphore& waitSemaphore, const uint32_t imageIndex) const
{
    return present(swapchain.getHandle(), waitSemaphore.getHandle(), imageIndex);
}

VkResult Queue::present(
    const VkSwapchainKHR swapchain, const VkSemaphore waitSemaphore, const uint32_t imageIndex) const
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    return vk->vkQueuePresentKHR(queue_, &presentInfo);
}

// -----------------------------------------------------------------------------------------------------------

VkResult Queue::present(
    const Swapchain& swapchain,
    const std::initializer_list<std::reference_wrapper<Semaphore>>& waitSemaphores,
    const uint32_t imageIndex) const
{
    std::vector<VkSemaphore> waitSemaphoreValues;
    waitSemaphoreValues.reserve(waitSemaphores.size());
    for(const auto& waitSemaphore : waitSemaphores)
    {
        waitSemaphoreValues.emplace_back(waitSemaphore.get().getHandle());
    }

    return present(swapchain.getHandle(), waitSemaphoreValues, imageIndex);
}

VkResult Queue::present(
    const VkSwapchainKHR& swapchain, const std::span<VkSemaphore>& waitSemaphores,
    const uint32_t imageIndex) const
{
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    presentInfo.pWaitSemaphores = waitSemaphores.data();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    return vk->vkQueuePresentKHR(queue_, &presentInfo);
}

} // namespace vkw