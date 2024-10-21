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

#include "vkWrappers/wrappers/extensions/InstanceExtensions.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#define INSTANTIATE_EXT_PROC(f)                                                                    \
    {                                                                                              \
        InstanceExt::f = (PFN_##f) vkGetInstanceProcAddr(instance, #f);                            \
        if(InstanceExt::f == nullptr)                                                              \
        {                                                                                          \
            return false;                                                                          \
        }                                                                                          \
    }

namespace vkw
{
static bool loadDebugUtils(VkInstance instance)
{
    INSTANTIATE_EXT_PROC(vkCmdBeginDebugUtilsLabelEXT);
    INSTANTIATE_EXT_PROC(vkCmdEndDebugUtilsLabelEXT);
    INSTANTIATE_EXT_PROC(vkCreateDebugUtilsMessengerEXT);
    INSTANTIATE_EXT_PROC(vkDestroyDebugUtilsMessengerEXT);
    INSTANTIATE_EXT_PROC(vkQueueBeginDebugUtilsLabelEXT);
    INSTANTIATE_EXT_PROC(vkQueueInsertDebugUtilsLabelEXT);
    INSTANTIATE_EXT_PROC(vkSetDebugUtilsObjectNameEXT);
    INSTANTIATE_EXT_PROC(vkSetDebugUtilsObjectTagEXT);
    INSTANTIATE_EXT_PROC(vkSubmitDebugUtilsMessageEXT);

    return true;
}

const char* getExtensionName(const InstanceExtension extName)
{
    switch(extName)
    {
        case DebugUtilsExt:
            return "VK_EXT_debug_utils";
        case SurfaceKhr:
            return "VK_KHR_surface";
        case XcbSurfaceKhr:
            return "VK_KHR_xcb_surface";
        case Win32SurfaceKhr:
            return "VK_KHR_win32_surface";
        default:
            return "";
    }
}

bool loadExtension(VkInstance instance, const InstanceExtension extName)
{
    switch(extName)
    {
        case DebugUtilsExt:
            CHECK_BOOL_RETURN_FALSE(loadDebugUtils(instance));
            break;
        case SurfaceKhr:
        case XcbSurfaceKhr:
        case Win32SurfaceKhr:
            break;
        case UnknownInstanceExtension:
            fprintf(stderr, "Unknown extension");
            return false;
    }

    return true;
}
} // namespace vkw

#undef INSTANTIATE_EXT_PROC