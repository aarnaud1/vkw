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

#include "vkWrappers/wrappers/extensions/DeviceExtensions.hpp"

#include "vkWrappers/wrappers/utils.hpp"

#include <string>

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

bool loadDeviceExtension(VkDevice device, const char* extName)
{
    const auto strName = std::string(extName);

    if(strName == VK_EXT_MESH_SHADER_EXTENSION_NAME)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(loadMeshShader(device));
    }

    return true;
}

} // namespace vkw
#undef INSTANTIATE_EXT_PROC
