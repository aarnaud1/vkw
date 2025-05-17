/*
 * Copyright (C) 2025 Adrien ARNAUD
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

#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/utils.hpp"

#include <limits>

namespace vkw
{
class Semaphore
{
  public:
    Semaphore() {}
    Semaphore(Device& device) { VKW_CHECK_BOOL_FAIL(this->init(device), "Creating semaphore"); }

    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&& cp) { *this = std::move(cp); }

    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore&& cp);

    ~Semaphore() { this->clear(); }

    bool init(Device& device);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkSemaphore& getHandle() { return semaphore_; }
    const VkSemaphore& getHandle() const { return semaphore_; }

  private:
    Device* device_{nullptr};
    VkSemaphore semaphore_{VK_NULL_HANDLE};

    bool initialized_{false};
};

class TimelineSemaphore
{
  public:
    TimelineSemaphore() {}
    TimelineSemaphore(Device& device, const uint64_t initValue = 0)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, initValue), "Creating semaphore");
    }

    TimelineSemaphore(const TimelineSemaphore&) = delete;
    TimelineSemaphore(TimelineSemaphore&& cp) { *this = std::move(cp); }

    TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
    TimelineSemaphore& operator=(TimelineSemaphore&& cp);

    ~TimelineSemaphore() { this->clear(); }

    bool init(Device& device, const uint64_t initValue = 0);

    void clear();

    bool isInitialized() const { return initialized_; }

    VkSemaphore& getHandle() { return semaphore_; }
    const VkSemaphore& getHandle() const { return semaphore_; }

    bool wait(const uint64_t waitValue, const uint64_t timeout = ~uint64_t(0))
    {
        VkSemaphoreWaitInfo waitInfo = {};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.pNext = nullptr;
        waitInfo.flags = 0;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &semaphore_;
        waitInfo.pValues = &waitValue;
        VKW_CHECK_VK_RETURN_FALSE(
            device_->vk().vkWaitSemaphores(device_->getHandle(), &waitInfo, timeout));

        return true;
    }

    bool signal(const uint64_t signalValue)
    {
        VkSemaphoreSignalInfo signalInfo = {};
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalInfo.pNext = nullptr;
        signalInfo.semaphore = semaphore_;
        signalInfo.value = signalValue;
        VKW_CHECK_VK_RETURN_FALSE(
            device_->vk().vkSignalSemaphore(device_->getHandle(), &signalInfo));

        return true;
    }

  private:
    Device* device_{nullptr};
    VkSemaphore semaphore_{VK_NULL_HANDLE};

    bool initialized_{false};
};

class Fence
{
  public:
    Fence() {}
    Fence(Device& device, const bool signaled = false)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, signaled), "Creating fence");
    }

    Fence(const Fence&) = delete;
    Fence(Fence&& cp) { *this = std::move(cp); }

    Fence& operator=(const Fence&) = delete;
    Fence& operator=(Fence&& cp);

    ~Fence() { this->clear(); }

    bool init(Device& device, const bool signaled = false);

    void clear();

    bool isInisitalized() const { return initialized_; }

    bool waitAndReset(const uint64_t timeout = std::numeric_limits<uint64_t>::max())
    {
        VKW_CHECK_BOOL_RETURN_FALSE(wait(timeout));
        VKW_CHECK_BOOL_RETURN_FALSE(reset());

        return true;
    }

    bool wait(const uint64_t timeout = std::numeric_limits<uint64_t>::max())
    {
        VKW_CHECK_VK_RETURN_FALSE(
            device_->vk().vkWaitForFences(device_->getHandle(), 1, &fence_, VK_TRUE, timeout));

        return true;
    }

    bool reset()
    {
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkResetFences(device_->getHandle(), 1, &fence_));

        return true;
    }

    static bool wait(
        const vkw::Device& device,
        const std::vector<Fence>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());
    static bool wait(
        const vkw::Device& device,
        const std::vector<Fence*>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());

    static bool waitAndReset(
        const vkw::Device& device,
        const std::vector<Fence>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());
    static bool waitAndReset(
        const vkw::Device& device,
        const std::vector<Fence*>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());

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
    Event(Device& device) { VKW_CHECK_BOOL_FAIL(this->init(device), "Creating event"); }

    Event(const Event&) = delete;
    Event(Event&& cp) { *this = std::move(cp); }

    Event& operator=(const Event&) = delete;
    Event& operator=(Event&& cp);

    ~Event() { this->clear(); }

    bool init(Device& device);

    void clear();

    VkEvent& getHandle() { return event_; }
    const VkEvent& getHandle() const { return event_; }

  private:
    Device* device_{nullptr};
    VkEvent event_{VK_NULL_HANDLE};
    bool initialized_{false};
};
} // namespace vkw