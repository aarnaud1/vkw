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

#include "vkWrappers/wrappers/Instance.hpp"

namespace vkw
{
class DebugMessenger
{
  public:
    DebugMessenger(){};
    DebugMessenger(Instance& instance);

    DebugMessenger(const DebugMessenger&) = delete;
    DebugMessenger(DebugMessenger&& cp);

    DebugMessenger& operator=(const DebugMessenger&) = delete;
    DebugMessenger& operator=(DebugMessenger&& cp);

    ~DebugMessenger();

    bool init(Instance& instance);

    void clear();

    bool isInitialized() const { return initialized_; }

  private:
    Instance* instance_{nullptr};
    VkDebugUtilsMessengerEXT messenger_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw