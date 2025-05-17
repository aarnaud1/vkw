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
#include "vkw/detail/utils.hpp"

namespace vkw
{
class Surface
{
  public:
    Surface() {}
    Surface(Instance& instance, VkSurfaceKHR&& surface);

    Surface(const Surface&) = delete;
    Surface(Instance&& rhs) { *this = std::move(rhs); }

    Surface& operator=(const Surface&) = delete;
    Surface& operator=(Surface&& rhs);

    ~Surface() { this->clear(); }

    bool initialized() const { return initialized_; }

    bool init(Instance& instance, VkSurfaceKHR&& surface);
    void clear();

    auto getHandle() const { return surface_; }

  private:
    Instance* instance_{nullptr};

    VkSurfaceKHR surface_{VK_NULL_HANDLE};
    bool initialized_{false};
};
} // namespace vkw