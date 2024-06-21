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

#include "vkWrappers/wrappers/CommandBuffer.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/QueueFamilies.hpp"
#include "vkWrappers/wrappers/Swapchain.hpp"
#include "vkWrappers/wrappers/Synchronization.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw
{
template <QueueFamilyType type>
class Queue
{
  public:
    Queue() {}
    Queue(Device &device) : queue_(device.getQueue(type)) {}

    Queue(const Queue &) = delete;
    Queue(Queue &&cp) { *this = std::move(cp); }

    Queue &operator=(const Queue &) = delete;
    Queue &operator=(Queue &&cp)
    {
        this->clear();

        std::swap(device_, cp.device_);
        std::swap(queue_, cp.queue_);

        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    ~Queue() { this->clear(); }

    void init(Device &device)
    {
        if(!initialized_)
        {
            device_ = &device;
            queue_ = device.getQueue(type);
            initialized_ = true;
        }
    }

    void clear()
    {
        device_ = nullptr;
        queue_ = VK_NULL_HANDLE;
        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    VkQueue &getHandle() { return queue_; }
    const VkQueue &getHandle() const { return queue_; }

    Queue &submit(CommandBuffer<type> &cmdBuffer, VkFence fence = VK_NULL_HANDLE)
    {
        VkSubmitInfo submitInfo
            = {VK_STRUCTURE_TYPE_SUBMIT_INFO,
               nullptr,
               0,
               nullptr,
               nullptr,
               1,
               &(cmdBuffer.getHandle()),
               0,
               nullptr};
        vkQueueSubmit(queue_, 1, &submitInfo, fence);
        return *this;
    }

    Queue &submit(
        CommandBuffer<type> &cmdBuffer,
        const std::vector<Semaphore *> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitFlags,
        const std::vector<Semaphore *> &signalSemaphores)
    {
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
               &(cmdBuffer.getHandle()),
               static_cast<uint32_t>(signalSemaphores.size()),
               signalSemaphoreValues.data()};
        vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);
        return *this;
    }

    Queue &submit(
        CommandBuffer<type> &cmdBuffer,
        const std::vector<Semaphore *> &waitSemaphores,
        const std::vector<VkPipelineStageFlags> &waitFlags,
        const std::vector<Semaphore *> &signalSemaphores,
        Fence &fence)
    {
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
               &(cmdBuffer.getHandle()),
               static_cast<uint32_t>(signalSemaphores.size()),
               signalSemaphoreValues.data()};
        vkQueueSubmit(queue_, 1, &submitInfo, fence.getHandle());
        return *this;
    }

    Queue &present(
        Swapchain &swapchain,
        const std::vector<Semaphore *> &waitSemaphores,
        const uint32_t imageIndex)
    {
        static_assert(
            type == QueueFamilyType::PRESENT,
            "Presentation can only be done with QueueFamily::TRANSFER type");

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

        vkQueuePresentKHR(queue_, &presentInfo);
        return *this;
    }

    Queue &waitIdle()
    {
        vkQueueWaitIdle(queue_);
        return *this;
    }

  private:
    Device *device_{nullptr};
    VkQueue queue_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
