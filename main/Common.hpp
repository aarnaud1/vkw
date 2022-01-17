/*
 * Copyright (C) 2022 Adrien ARNAUD
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

#include <vulkan/vulkan.h>

// -----------------------------------------------------------------------------

static constexpr vk::BufferPropertyFlags hostStagingFlags = {
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

static constexpr vk::BufferPropertyFlags deviceFlags = {
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

static constexpr vk::BufferPropertyFlags uniformDeviceFlags = {
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

static constexpr vk::BufferPropertyFlags uniformHostStagingFlags = {
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

static constexpr vk::ImagePropertyFlags imgDeviceFlags = {
    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

// -----------------------------------------------------------------------------

template <typename T>
static inline T randVal()
{
  return 2.0f * float(rand()) / float(RAND_MAX) - 1.0f;
}

template <typename T>
static std::vector<T> randArray(const size_t size)
{
  std::vector<T> ret(size);
  for(size_t i = 0; i < size; i++)
  {
    ret[i] = randVal<T>();
  }
  return ret;
}

template <typename T>
static bool compareArrays(std::vector<T> &v0, std::vector<T> &v1)
{
  assert(v0.size() == v1.size());
  for(size_t i = 0; i < v0.size(); i++)
  {
    if(v0[i] != v1[i])
    {
      return false;
    }
  }
  return true;
}
