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

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vulkan/vulkan.h>

namespace vkw
{
class IMemoryObject
{
  public:
    virtual ~IMemoryObject() {}

    VkDeviceSize getMemSize() const { return memSize_; }
    VkDeviceSize getMemAlign() const { return memAlign_; }
    VkDeviceSize getMemOffset() const { return memOffset_; }

  protected:
    virtual bool bindResource(VkDeviceMemory mem, const VkDeviceSize offset) = 0;

    friend class Memory;

    VkDeviceSize memAlign_{0};
    VkDeviceSize memSize_{0};
    VkDeviceSize memOffset_{0};

    uint32_t memTypeBits_{0};
};

} // namespace vkw
