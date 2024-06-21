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

#include <limits>
#include <vulkan/vulkan.h>

namespace vkw
{
class Semaphore
{
  public:
    Semaphore() {}
    Semaphore(Device& device) : device_{&device} { this->init(device); }

    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&& cp) { *this = std::move(cp); }

    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore&& cp)
    {
        this->clear();
        std::swap(device_, cp.device_);
        std::swap(semaphore_, cp.semaphore_);
        std::swap(initialized_, cp.initialized_);
        return *this;
    }

    ~Semaphore() { this->clear(); }

    void init(Device& device)
    {
        if(!initialized_)
        {
            device_ = &device;

            VkSemaphoreCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;

            CHECK_VK(
                vkCreateSemaphore(device_->getHandle(), &createInfo, nullptr, &semaphore_),
                "Creating semaphore");

            initialized_ = true;
        }
    }

    void clear()
    {
        if(semaphore_ != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(device_->getHandle(), semaphore_, nullptr);
        }

        device_ = nullptr;
        semaphore_ = VK_NULL_HANDLE;
        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    VkSemaphore& getHandle() { return semaphore_; }
    const VkSemaphore& getHandle() const { return semaphore_; }

  private:
    Device* device_{nullptr};
    VkSemaphore semaphore_{VK_NULL_HANDLE};

    bool initialized_{false};
};

class Fence
{
  public:
    Fence() {}
    Fence(Device& device, const bool signaled = false) { this->init(device, signaled); }

    Fence(const Fence&) = delete;
    Fence(Fence&& cp) { *this = std::move(cp); }

    Fence& operator=(const Fence&) = delete;
    Fence& operator=(Fence&& cp)
    {
        this->clear();
        std::swap(cp.device_, device_);
        std::swap(cp.fence_, fence_);
        std::swap(cp.initialized_, initialized_);
        return *this;
    }

    ~Fence() { this->clear(); }

    void init(Device& device, const bool signaled = false)
    {
        if(!initialized_)
        {
            device_ = &device;
            VkFenceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

            CHECK_VK(
                vkCreateFence(device_->getHandle(), &createInfo, nullptr, &fence_),
                "Creating fence");
        }

        initialized_ = true;
    }

    void clear()
    {
        if(fence_ != VK_NULL_HANDLE)
        {
            vkDestroyFence(device_->getHandle(), fence_, nullptr);
        }

        device_ = nullptr;
        fence_ = VK_NULL_HANDLE;
        initialized_ = false;
    }

    bool isInisitalized() const { return initialized_; }

    void waitAndReset(const uint64_t timeout = std::numeric_limits<uint64_t>::max())
    {
        wait(timeout);
        reset();
    }

    void wait(const uint64_t timeout = std::numeric_limits<uint64_t>::max())
    {
        vkWaitForFences(device_->getHandle(), 1, &fence_, VK_TRUE, timeout);
    }

    void reset() { vkResetFences(device_->getHandle(), 1, &fence_); }

    VkFence& getHandle() { return fence_; }
    const VkFence& getHandle() const { return fence_; }

  private:
    Device* device_{nullptr};
    VkFence fence_{VK_NULL_HANDLE};
    bool initialized_{false};
};

class Event
{
  public:
    Event() {}
    Event(Device& device) { this->init(device); }

    Event(const Event&) = delete;
    Event(Event&& cp) { *this = std::move(cp); }

    Event& operator=(const Event&) = delete;
    Event& operator=(Event&& cp)
    {
        this->clear();
        std::swap(cp.device_, device_);
        std::swap(cp.event_, event_);
        std::swap(cp.initialized_, initialized_);
        return *this;
    }

    ~Event() { this->clear(); }

    void init(Device& device)
    {
        if(!initialized_)
        {
            device_ = &device;

            VkEventCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
            createInfo.pNext = nullptr;
            createInfo.flags = 0;

            CHECK_VK(
                vkCreateEvent(device_->getHandle(), &createInfo, nullptr, &event_),
                "Creating event");

            initialized_ = true;
        }
    }

    void clear()
    {
        if(event_ != VK_NULL_HANDLE)
        {
            vkDestroyEvent(device_->getHandle(), event_, nullptr);
        }
    }

    VkEvent& getHandle() { return event_; }
    const VkEvent& getHandle() const { return event_; }

  private:
    Device* device_{nullptr};
    VkEvent event_{VK_NULL_HANDLE};
    bool initialized_{false};
};
} // namespace vkw