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

#include "vulkan/vulkan.h"

namespace vk
{
enum ImageFormat
{
    R,
    RG,
    RGB,
    RGBA
};

template <ImageFormat imgFormat, typename T>
struct FormatType
{};

template <>
struct FormatType<ImageFormat::R, float>
{
    static constexpr VkFormat format = VK_FORMAT_R32_SFLOAT;
};

template <>
struct FormatType<ImageFormat::RG, float>
{
    static constexpr VkFormat format = VK_FORMAT_R32G32_SFLOAT;
};

template <>
struct FormatType<ImageFormat::RGB, float>
{
    static constexpr VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
};

template <>
struct FormatType<ImageFormat::RGBA, float>
{
    static constexpr VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
};

template <>
struct FormatType<ImageFormat::R, uint32_t>
{
    static constexpr VkFormat format = VK_FORMAT_R32_UINT;
};

template <>
struct FormatType<ImageFormat::RG, uint32_t>
{
    static constexpr VkFormat format = VK_FORMAT_R32G32_UINT;
};

template <>
struct FormatType<ImageFormat::RGB, uint32_t>
{
    static constexpr VkFormat format = VK_FORMAT_R32G32B32_UINT;
};

template <>
struct FormatType<ImageFormat::RGBA, uint32_t>
{
    static constexpr VkFormat format = VK_FORMAT_R32G32B32A32_UINT;
};
} // namespace vk
