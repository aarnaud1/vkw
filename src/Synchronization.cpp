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

#include "vkw/detail/Synchronization.hpp"

namespace vkw
{
Semaphore& Semaphore::operator=(Semaphore&& cp)
{
    this->clear();
    std::swap(device_, cp.device_);
    std::swap(semaphore_, cp.semaphore_);
    std::swap(initialized_, cp.initialized_);
    return *this;
}

bool Semaphore::init(const Device& device)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    VKW_INIT_CHECK_VK(
        device_->vk().vkCreateSemaphore(device_->getHandle(), &createInfo, nullptr, &semaphore_));

    initialized_ = true;

    return true;
}

void Semaphore::clear()
{
    VKW_DELETE_VK(Semaphore, semaphore_);

    device_ = nullptr;
    initialized_ = false;
}

// -------------------------------------------------------------------------------------------------

TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& cp)
{
    this->clear();
    std::swap(device_, cp.device_);
    std::swap(semaphore_, cp.semaphore_);
    std::swap(initialized_, cp.initialized_);
    return *this;
}

bool TimelineSemaphore::init(const Device& device, const uint64_t initValue)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;

    VkSemaphoreTypeCreateInfo timelineCreateInfo = {};
    timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineCreateInfo.pNext = nullptr;
    timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineCreateInfo.initialValue = initValue;

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = &timelineCreateInfo;
    createInfo.flags = 0;
    VKW_INIT_CHECK_VK(
        device_->vk().vkCreateSemaphore(device_->getHandle(), &createInfo, nullptr, &semaphore_));

    initialized_ = true;

    return true;
}

void TimelineSemaphore::clear()
{
    VKW_DELETE_VK(Semaphore, semaphore_);

    device_ = nullptr;
    initialized_ = false;
}

// -------------------------------------------------------------------------------------------------

Fence& Fence::operator=(Fence&& cp)
{
    this->clear();
    std::swap(cp.device_, device_);
    std::swap(cp.fence_, fence_);
    std::swap(cp.initialized_, initialized_);
    return *this;
}

bool Fence::init(const Device& device, const bool signaled)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VKW_INIT_CHECK_VK(
        device_->vk().vkCreateFence(device_->getHandle(), &createInfo, nullptr, &fence_));

    initialized_ = true;

    return true;
}

void Fence::clear()
{
    VKW_DELETE_VK(Fence, fence_);

    device_ = nullptr;
    initialized_ = false;
}

bool Fence::wait(
    const vkw::Device& device, const std::vector<Fence>& fences, const uint64_t timeout)
{
    if(fences.empty())
    {
        return true;
    }

    std::vector<VkFence> fenceList;
    for(const auto& fence : fences)
    {
        fenceList.push_back(fence.getHandle());
    }

    VKW_CHECK_VK_RETURN_FALSE(device.vk().vkWaitForFences(
        device.getHandle(),
        static_cast<uint32_t>(fenceList.size()),
        fenceList.data(),
        VK_TRUE,
        timeout));

    return true;
}

bool Fence::wait(
    const vkw::Device& device, const std::vector<Fence*>& fences, const uint64_t timeout)
{
    if(fences.empty())
    {
        return true;
    }

    std::vector<VkFence> fenceList;
    for(const auto* fence : fences)
    {
        fenceList.push_back(fence->getHandle());
    }

    VKW_CHECK_VK_RETURN_FALSE(device.vk().vkWaitForFences(
        device.getHandle(),
        static_cast<uint32_t>(fenceList.size()),
        fenceList.data(),
        VK_TRUE,
        timeout));

    return true;
}

bool Fence::waitAndReset(
    const vkw::Device& device, const std::vector<Fence>& fences, const uint64_t timeout)
{
    if(fences.empty())
    {
        return true;
    }

    std::vector<VkFence> fenceList;
    for(const auto& fence : fences)
    {
        fenceList.push_back(fence.getHandle());
    }

    VKW_CHECK_VK_RETURN_FALSE(device.vk().vkWaitForFences(
        device.getHandle(),
        static_cast<uint32_t>(fenceList.size()),
        fenceList.data(),
        VK_TRUE,
        timeout));
    VKW_CHECK_VK_RETURN_FALSE(device.vk().vkResetFences(
        device.getHandle(), static_cast<uint32_t>(fenceList.size()), fenceList.data()));

    return true;
}
bool Fence::waitAndReset(
    const vkw::Device& device, const std::vector<Fence*>& fences, const uint64_t timeout)
{
    if(fences.empty())
    {
        return true;
    }

    std::vector<VkFence> fenceList;
    for(const auto* fence : fences)
    {
        fenceList.push_back(fence->getHandle());
    }

    VKW_CHECK_VK_RETURN_FALSE(device.vk().vkWaitForFences(
        device.getHandle(),
        static_cast<uint32_t>(fenceList.size()),
        fenceList.data(),
        VK_TRUE,
        timeout));
    VKW_CHECK_VK_RETURN_FALSE(device.vk().vkResetFences(
        device.getHandle(), static_cast<uint32_t>(fenceList.size()), fenceList.data()));

    return true;
}

// -------------------------------------------------------------------------------------------------

Event& Event::operator=(Event&& cp)
{
    this->clear();
    std::swap(cp.device_, device_);
    std::swap(cp.event_, event_);
    std::swap(cp.initialized_, initialized_);
    return *this;
}

bool Event::init(const Device& device)
{
    VKW_ASSERT(this->initialized() == false);

    device_ = &device;

    VkEventCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    VKW_INIT_CHECK_VK(
        device_->vk().vkCreateEvent(device_->getHandle(), &createInfo, nullptr, &event_));

    initialized_ = true;

    return true;
}

void Event::clear()
{
    VKW_DELETE_VK(Event, event_);

    device_ = nullptr;
    initialized_ = false;
}
} // namespace vkw