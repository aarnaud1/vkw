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

#include "vkw/vkw.hpp"

namespace vkw
{
// -------------------------------------------------------------------------------------------------
// -------------------------- Various buffer types -------------------------------------------------
// -------------------------------------------------------------------------------------------------

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

// -------------------------------------------------------------------------------------------------
// -------------------------- Utility buffer types -------------------------------------------------
// -------------------------------------------------------------------------------------------------

template <typename T>
using AccelerationStructureGeometryBuffer = Buffer<
    T, MemoryType::HostDevice,
    VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR>;
using AccelerationStructureSratchBuffer = Buffer<
    uint8_t, MemoryType::HostDevice,
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT>;

// -------------------------------------------------------------------------------------------------
// -------------------------- Various image types --------------------------------------------------
// -------------------------------------------------------------------------------------------------

using InputAttachment = Image<MemoryType::Device, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT>;
using RenderImage = Image<MemoryType::Device, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT>;
using DepthImage = Image<MemoryType::Device, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT>;
using StorageImage = Image<MemoryType::HostDevice, VK_IMAGE_USAGE_STORAGE_BIT>;
using Texture = Image<MemoryType::HostDevice, VK_IMAGE_USAGE_SAMPLED_BIT>;
} // namespace vkw