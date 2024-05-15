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

#include "vkWrappers/wrappers/utils.hpp"

#include <string>
#include <fstream>

namespace vk
{
namespace utils
{
    VkShaderModule createShaderModule(const VkDevice device, const std::vector<char> &src)
    {
        VkShaderModule ret;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = src.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(src.data());

        CHECK_VK(
            vkCreateShaderModule(device, &createInfo, nullptr, &ret), "Creating shader module");

        return ret;
    }

    std::vector<char> readShader(const std::string &shaderSource)
    {
        std::ifstream file(shaderSource, std::ios::ate | std::ios::binary);

        if(!file.is_open())
        {
            fprintf(stderr, "Error opening file %s\n", shaderSource.c_str());
            exit(1);
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
} // namespace utils
} // namespace vk