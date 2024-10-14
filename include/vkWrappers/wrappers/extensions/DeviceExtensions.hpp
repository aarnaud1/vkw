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

#include <vulkan/vulkan.h>

namespace vkw
{
// List of supported device extensions
enum DeviceExtension
{
    SwapchainKhr = 0,           // VK_KHR_swapchain
    ExternalMemoryKhr = 1,      // VK_KHR_external_memory
    ExternalMemoryFdKhr = 2,    // VK_KHR_external_memory_fd
    ExternalSemaphoreKhr = 3,   // VK_KHR_external_semaphore
    ExternalSemaphoreFdKhr = 5, // VK_KHR_external_semaphore_fd
    UnknownDeviceExtension = 6
};

const char* getExtensionName(const DeviceExtension extName);

bool loadExtension(VkDevice device, const DeviceExtension extName);

#define PFN(f)              PFN_##f
#define VARNAME(f)          f
#define DECLARE_EXT_PROC(f) static inline PFN(f) VARNAME(f) = nullptr
struct DeviceExt
{
    // VK_KHR_external_memory_fd
    DECLARE_EXT_PROC(vkGetMemoryFdKHR);
    DECLARE_EXT_PROC(vkGetMemoryFdPropertiesKHR);

    // VK_KHR_external_semaphore_fd
    DECLARE_EXT_PROC(vkGetSemaphoreFdKHR);
    DECLARE_EXT_PROC(vkImportSemaphoreFdKHR);
}; // namespace ext
#undef DECLARE_EXT_PROC
#undef VARNAME
#undef PFN
} // namespace vkw