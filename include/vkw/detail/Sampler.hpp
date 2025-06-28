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
#include "vkw/detail/Device.hpp"
#include "vkw/detail/utils.hpp"

namespace vkw
{
class Sampler final
{
  public:
    Sampler() = default;

    Sampler(const Device& device, const VkSamplerCreateInfo& createInfo)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, createInfo), "Error creating sampler");
    }

    Sampler(const Sampler&) = delete;
    Sampler(Sampler&& rhs) { *this = std::move(rhs); }

    Sampler& operator=(const Sampler&) = delete;
    Sampler& operator=(Sampler&& rhs)
    {
        this->clear();

        std::swap(device_, rhs.device_);
        std::swap(sampler_, rhs.sampler_);
        std::swap(initialized_, rhs.initialized_);

        return *this;
    }

    ~Sampler() { this->clear(); }

    bool initialized() const { return initialized_; }

    VkSampler getHandle() const { return sampler_; }

    bool init(const Device& device, const VkSamplerCreateInfo& createInfo)
    {
        if(!initialized_)
        {
            device_ = &device;
            VKW_INIT_CHECK_VK(vkCreateSampler(device_->getHandle(), &createInfo, nullptr, &sampler_));
            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        VKW_DELETE_VK(Sampler, sampler_);

        device_ = nullptr;
        initialized_ = false;
    }

  private:
    const Device* device_{nullptr};

    VkSampler sampler_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw