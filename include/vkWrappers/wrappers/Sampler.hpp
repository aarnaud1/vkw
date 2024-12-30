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
#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
class Sampler final
{
  public:
    Sampler() = default;

    Sampler(Device& device, const VkSamplerCreateInfo& createInfo)
    {
        VKW_CHECK_BOOL_THROW(this->init(device, createInfo), "Error creating sampler");
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

    bool isInitialized() const { return initialized_; }

    VkSampler getHandle() const { return sampler_; }

    bool init(Device& device, const VkSamplerCreateInfo& createInfo)
    {
        if(!initialized_)
        {
            device_ = &device;
            VKW_INIT_CHECK_VK(
                vkCreateSampler(device_->getHandle(), &createInfo, nullptr, &sampler_));
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
    Device* device_{nullptr};

    VkSampler sampler_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw