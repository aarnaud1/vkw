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

#include "vkWrappers/wrappers/extensions/DeviceExtensions.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#define INSTANTIATE_EXT_PROC(f)                                                                    \
    {                                                                                              \
        DeviceExt::f = (PFN_##f) vkGetDeviceProcAddr(device, #f);                                  \
        if(DeviceExt::f == nullptr)                                                                \
        {                                                                                          \
            return false;                                                                          \
        }                                                                                          \
    }

namespace vkw
{
static bool loadMeshShader(VkDevice device)
{
    INSTANTIATE_EXT_PROC(vkCmdDrawMeshTasksEXT);
    INSTANTIATE_EXT_PROC(vkCmdDrawMeshTasksIndirectCountEXT);
    INSTANTIATE_EXT_PROC(vkCmdDrawMeshTasksIndirectEXT);

    return true;
}

const char* getExtensionName(const DeviceExtension extName)
{
    switch(extName)
    {
        case SwapchainKhr:
            return "VK_KHR_swapchain";
        case MeshShaderExt:
            return "VK_EXT_mesh_shader";
        default:
            return "";
    }
}

bool loadExtension(VkDevice device, const DeviceExtension extName)
{
    switch(extName)
    {
        case MeshShaderExt:
            VKW_CHECK_BOOL_RETURN_FALSE(loadMeshShader(device));
            break;
        case SwapchainKhr:
            break;
        case UnknownDeviceExtension:
            fprintf(stderr, "Unknown extension");
            return false;
    }

    return true;
}

} // namespace vkw
#undef INSTANTIATE_EXT_PROC
