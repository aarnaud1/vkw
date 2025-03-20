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

#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
class Surface
{
  public:
    Surface() {}
    Surface(Instance& instance, VkSurfaceKHR&& surface)
    {
        VKW_CHECK_BOOL_THROW(
            this->init(instance, std::move(surface)), "Error initializing surface object");
    }

    Surface(const Surface&) = delete;
    Surface(Instance&& rhs) { *this = std::move(rhs); }

    Surface& operator=(const Surface&) = delete;
    Surface& operator=(Surface&& rhs)
    {
        this->clear();

        std::swap(instance_, rhs.instance_);
        std::swap(surface_, rhs.surface_);
        std::swap(initialized_, rhs.initialized_);

        return *this;
    }

    ~Surface() { this->clear(); }

    bool initialized() const { return initialized_; }

    bool init(Instance& instance, VkSurfaceKHR&& surface)
    {
        if(!initialized_)
        {
            instance_ = &instance;
            std::swap(surface_, surface);
            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        if(surface_ != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance_->getHandle(), surface_, nullptr);
            surface_ = VK_NULL_HANDLE;
        }

        instance_ = nullptr;
        initialized_ = false;
    }

    auto getHandle() const { return surface_; }

  private:
    Instance* instance_{nullptr};

    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    bool initialized_{false};
};
} // namespace vkw