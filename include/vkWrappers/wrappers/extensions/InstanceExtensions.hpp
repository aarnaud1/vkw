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
// List of supported instance extensions
enum InstanceExtension
{
    DebugUtilsExt = 0,   // VK_KHR_debug_utils
    SurfaceKhr = 1,      // VK_KHR_surface
    XcbSurfaceKhr = 2,   // VK_KHR_xcb_surface
    Win32SurfaceKhr = 3, // VK_KHR_win32_surface
    UnknownInstanceExtension = 4
};

const char* getExtensionName(const InstanceExtension extName);

bool loadExtension(VkInstance instance, const InstanceExtension extName);

#define PFN(f)              PFN_##f
#define VARNAME(f)          f
#define DECLARE_EXT_PROC(f) static inline PFN(f) VARNAME(f) = nullptr
struct InstanceExt
{
    // VK_EXT_debug_utils
    DECLARE_EXT_PROC(vkCmdBeginDebugUtilsLabelEXT);
    DECLARE_EXT_PROC(vkCmdEndDebugUtilsLabelEXT);
    DECLARE_EXT_PROC(vkCreateDebugUtilsMessengerEXT);
    DECLARE_EXT_PROC(vkDestroyDebugUtilsMessengerEXT);
    DECLARE_EXT_PROC(vkQueueBeginDebugUtilsLabelEXT);
    DECLARE_EXT_PROC(vkQueueInsertDebugUtilsLabelEXT);
    DECLARE_EXT_PROC(vkSetDebugUtilsObjectNameEXT);
    DECLARE_EXT_PROC(vkSetDebugUtilsObjectTagEXT);
    DECLARE_EXT_PROC(vkSubmitDebugUtilsMessageEXT);
}; // namespace ext
#undef DECLARE_EXT_PROC
#undef VARNAME
#undef PFN
} // namespace vkw