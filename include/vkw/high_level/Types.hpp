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

#include "vkw/vkw.hpp"

namespace vkw
{
// Various buffer types
template <typename T>
using VertexBuffer = Buffer<T, MemoryType::HostDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT>;

template <typename T>
using IndexBuffer = Buffer<T, MemoryType::HostDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT>;

template <typename T>
using StorageBuffer = Buffer<T, MemoryType::HostDevice, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT>;

template <typename T>
using ConstantBuffer = Buffer<T, MemoryType::Device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT>;

template <typename T>
using UniformBuffer = Buffer<T, MemoryType::HostStaging, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT>;

template <typename T>
using AccelerationStructureGeometryBuffer = Buffer<
    T,
    MemoryType::HostDevice,
    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR>;

using AccelerationStructureSratchBuffer = Buffer<
    uint8_t,
    MemoryType::HostDevice,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT>;

// Various image types
using RenderImage = Image<MemoryType::Device, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT>;
using DepthImage = Image<MemoryType::Device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT>;
using StorageImage = Image<MemoryType::HostDevice, VK_IMAGE_USAGE_STORAGE_BIT>;
} // namespace vkw