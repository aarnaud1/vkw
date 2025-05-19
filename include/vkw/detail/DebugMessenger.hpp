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
#include "vkw/detail/Instance.hpp"

namespace vkw
{
class DebugMessenger
{
  public:
    DebugMessenger() {};
    DebugMessenger(Instance& instance);

    DebugMessenger(const DebugMessenger&) = delete;
    DebugMessenger(DebugMessenger&& cp);

    DebugMessenger& operator=(const DebugMessenger&) = delete;
    DebugMessenger& operator=(DebugMessenger&& cp);

    ~DebugMessenger();

    bool init(Instance& instance);

    void clear();

    bool initialized() const { return initialized_; }

  private:
    Instance* instance_{nullptr};
    VkDebugUtilsMessengerEXT messenger_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw