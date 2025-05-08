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

#include "vkw/wrappers/Synchronization.hpp"

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

bool Semaphore::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;

        VkSemaphoreCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        VKW_INIT_CHECK_VK(device_->vk().vkCreateSemaphore(
            device_->getHandle(), &createInfo, nullptr, &semaphore_));

        initialized_ = true;
    }

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

bool TimelineSemaphore::init(Device& device, const uint64_t initValue)
{
    if(!initialized_)
    {
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
        VKW_INIT_CHECK_VK(device_->vk().vkCreateSemaphore(
            device_->getHandle(), &createInfo, nullptr, &semaphore_));

        initialized_ = true;
    }

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

bool Fence::init(Device& device, const bool signaled)
{
    if(!initialized_)
    {
        device_ = &device;
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
        VKW_INIT_CHECK_VK(
            device_->vk().vkCreateFence(device_->getHandle(), &createInfo, nullptr, &fence_));

        initialized_ = true;
    }

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

bool Event::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;

        VkEventCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        VKW_INIT_CHECK_VK(
            device_->vk().vkCreateEvent(device_->getHandle(), &createInfo, nullptr, &event_));

        initialized_ = true;
    }

    return true;
}

void Event::clear()
{
    VKW_DELETE_VK(Event, event_);

    device_ = nullptr;
    initialized_ = false;
}
} // namespace vkw