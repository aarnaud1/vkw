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

#include "vkw/detail/Surface.hpp"

namespace vkw
{
Surface::Surface(Instance& instance, VkSurfaceKHR&& surface)
{
    VKW_CHECK_BOOL_FAIL(
        this->init(instance, std::move(surface)), "Error initializing surface object");
}

Surface& Surface::operator=(Surface&& rhs)
{
    this->clear();

    std::swap(instance_, rhs.instance_);
    std::swap(surface_, rhs.surface_);
    std::swap(initialized_, rhs.initialized_);

    return *this;
}

bool Surface::init(Instance& instance, VkSurfaceKHR&& surface)
{
    VKW_ASSERT(this->initialized() == false);

    instance_ = &instance;
    std::swap(surface_, surface);
    initialized_ = true;

    return true;
}

void Surface::clear()
{
    if(surface_ != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance_->getHandle(), surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    instance_ = nullptr;
    initialized_ = false;
}
} // namespace vkw