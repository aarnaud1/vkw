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
#include "vkw/detail/utils.hpp"

#include <limits>

namespace vkw
{
class Semaphore
{
  public:
    constexpr Semaphore() {}
    Semaphore(const Device& device) { VKW_CHECK_BOOL_FAIL(this->init(device), "Creating semaphore"); }

    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&& rhs) { *this = std::move(rhs); }

    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore&& rhs);

    ~Semaphore() { this->clear(); }

    bool init(const Device& device);

    void clear();

    bool initialized() const { return initialized_; }

    VkSemaphore& getHandle() { return semaphore_; }
    const VkSemaphore& getHandle() const { return semaphore_; }

  private:
    const Device* device_{nullptr};
    VkSemaphore semaphore_{VK_NULL_HANDLE};

    bool initialized_{false};
};

class TimelineSemaphore
{
  public:
    constexpr TimelineSemaphore() {}
    TimelineSemaphore(const Device& device, const uint64_t initValue = 0)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, initValue), "Creating semaphore");
    }

    TimelineSemaphore(const TimelineSemaphore&) = delete;
    TimelineSemaphore(TimelineSemaphore&& rhs) { *this = std::move(rhs); }

    TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
    TimelineSemaphore& operator=(TimelineSemaphore&& rhs);

    ~TimelineSemaphore() { this->clear(); }

    bool init(const Device& device, const uint64_t initValue = 0);

    void clear();

    bool initialized() const { return initialized_; }

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
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkWaitSemaphores(device_->getHandle(), &waitInfo, timeout));

        return true;
    }

    bool signal(const uint64_t signalValue)
    {
        VkSemaphoreSignalInfo signalInfo = {};
        signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalInfo.pNext = nullptr;
        signalInfo.semaphore = semaphore_;
        signalInfo.value = signalValue;
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkSignalSemaphore(device_->getHandle(), &signalInfo));

        return true;
    }

  private:
    const Device* device_{nullptr};
    VkSemaphore semaphore_{VK_NULL_HANDLE};

    bool initialized_{false};
};

class Fence
{
  public:
    constexpr Fence() {}
    Fence(const Device& device, const bool signaled = false)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, signaled), "Creating fence");
    }

    Fence(const Fence&) = delete;
    Fence(Fence&& rhs) { *this = std::move(rhs); }

    Fence& operator=(const Fence&) = delete;
    Fence& operator=(Fence&& rhs);

    ~Fence() { this->clear(); }

    bool init(const Device& device, const bool signaled = false);

    void clear();

    bool initialized() const { return initialized_; }

    bool waitAndReset(const uint64_t timeout = std::numeric_limits<uint64_t>::max()) const
    {
        VKW_CHECK_BOOL_RETURN_FALSE(wait(timeout));
        VKW_CHECK_BOOL_RETURN_FALSE(reset());

        return true;
    }

    bool wait(const uint64_t timeout = std::numeric_limits<uint64_t>::max()) const
    {
        VKW_CHECK_VK_RETURN_FALSE(
            device_->vk().vkWaitForFences(device_->getHandle(), 1, &fence_, VK_TRUE, timeout));

        return true;
    }

    bool reset() const
    {
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkResetFences(device_->getHandle(), 1, &fence_));

        return true;
    }

    VkResult getStatus() const { return device_->vk().vkGetFenceStatus(device_->getHandle(), fence_); }

    static bool wait(
        const vkw::Device& device, const std::vector<Fence>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());
    static bool wait(
        const vkw::Device& device, const std::vector<Fence*>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());

    static bool waitAndReset(
        const vkw::Device& device, const std::vector<Fence>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());
    static bool waitAndReset(
        const vkw::Device& device, const std::vector<Fence*>& fences,
        const uint64_t timeout = std::numeric_limits<uint64_t>::max());

    VkFence& getHandle() { return fence_; }
    const VkFence& getHandle() const { return fence_; }

  private:
    const Device* device_{nullptr};
    VkFence fence_{VK_NULL_HANDLE};
    bool initialized_{false};
};

class Event
{
  public:
    constexpr Event() {}
    Event(const Device& device) { VKW_CHECK_BOOL_FAIL(this->init(device), "Creating event"); }

    Event(const Event&) = delete;
    Event(Event&& rhs) { *this = std::move(rhs); }

    Event& operator=(const Event&) = delete;
    Event& operator=(Event&& rhs);

    ~Event() { this->clear(); }

    bool init(const Device& device);

    void clear();

    bool initialized() const { return initialized_; }

    VkEvent& getHandle() { return event_; }
    const VkEvent& getHandle() const { return event_; }

  private:
    const Device* device_{nullptr};
    VkEvent event_{VK_NULL_HANDLE};
    bool initialized_{false};
};
} // namespace vkw
