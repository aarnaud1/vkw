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

#include "vkw/detail/CommandBuffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/Instance.hpp"
#include "vkw/detail/Queue.hpp"
#include "vkw/detail/utils.hpp"

#include <vector>

namespace vkw
{
class CommandPool
{
  public:
    CommandPool() {}
    CommandPool(
        const Device& device, const Queue& queue,
        const VkCommandPoolCreateFlags flags
        = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, queue, flags), "Initializing command pool");
    }

    CommandPool(const CommandPool&) = delete;
    CommandPool(CommandPool&& cp) { *this = std::move(cp); }

    CommandPool& operator=(const CommandPool&) = delete;
    CommandPool& operator=(CommandPool&& cp)
    {
        this->clear();

        std::swap(device_, cp.device_);
        std::swap(commandPool_, cp.commandPool_);
        std::swap(initialized_, cp.initialized_);

        return *this;
    }

    ~CommandPool() { this->clear(); }

    bool init(
        const Device& device, const Queue& queue,
        const VkCommandPoolCreateFlags flags
        = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
    {
        VKW_ASSERT(this->initialized() == false);

        device_ = &device;

        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = flags;
        createInfo.queueFamilyIndex = queue.queueFamilyIndex();
        VKW_INIT_CHECK_VK(
            device_->vk().vkCreateCommandPool(device_->getHandle(), &createInfo, nullptr, &commandPool_));

        initialized_ = true;

        return true;
    }

    void clear()
    {
        VKW_DELETE_VK(CommandPool, commandPool_);

        device_ = nullptr;
        initialized_ = false;
    }

    bool initialized() const { return initialized_; }

    CommandBuffer createCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const
    {
        VKW_ASSERT(this->initialized());

        CommandBuffer cmdBuffer{};
        if(!cmdBuffer.init(*device_, commandPool_, level)) { return {}; }
        return cmdBuffer;
    }

    std::vector<CommandBuffer> createCommandBuffers(
        const size_t n, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const
    {
        VKW_ASSERT(this->initialized());
        std::vector<CommandBuffer> ret;
        for(size_t i = 0; i < n; ++i)
        {
            CommandBuffer cmdBuffer{};
            if(!cmdBuffer.init(*device_, commandPool_, level))
            {
                ret.clear();
                return {};
            }
            ret.emplace_back(std::move(cmdBuffer));
        }
        return ret;
    }

    VkCommandPool& getHandle() { return commandPool_; }
    const VkCommandPool& getHandle() const { return commandPool_; }

  private:
    const Device* device_{nullptr};
    VkCommandPool commandPool_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw
