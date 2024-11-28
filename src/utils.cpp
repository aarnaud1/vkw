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

#include <fstream>
#include <string>

namespace vkw
{
namespace utils
{
    VkShaderModule createShaderModule(const VkDevice device, const std::vector<char>& src)
    {
        VkShaderModule ret;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = src.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(src.data());

        VKW_CHECK_VK_THROW(
            vkCreateShaderModule(device, &createInfo, nullptr, &ret), "Creating shader module");

        return ret;
    }

    std::vector<char> readShader(const std::string& shaderSource)
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

    uint32_t findMemoryType(
        const VkPhysicalDevice physicalDevice,
        const VkMemoryPropertyFlags requiredFlags,
        const VkMemoryPropertyFlags preferredFlags,
        const VkMemoryPropertyFlags undesiredFlags,
        const VkMemoryRequirements requirements)
    {
        static constexpr uint32_t notFoundIndex = ~uint32_t(0);

        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        uint32_t index = notFoundIndex;

        // Check without undesired property flags
        for(uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if((requirements.memoryTypeBits & (1 << i)) == 0)
            {
                // Incompatible memory type
                continue;
            }

            const auto& props = memProperties.memoryTypes[i];
            if(props.propertyFlags & undesiredFlags != 0)
            {
                continue;
            }

            if(((props.propertyFlags & requiredFlags) == requiredFlags)
               && ((index == notFoundIndex)
                   || (props.propertyFlags & preferredFlags) == preferredFlags))
            {
                if(memProperties.memoryHeaps[props.heapIndex].size
                   >= alignedSize(requirements.size, requirements.alignment))
                {
                    index = i;
                }
            }
        }

        // Do the search again with undesired flags
        if(index == notFoundIndex)
        {
            for(uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
            {
                if((requirements.memoryTypeBits & (1 << i)) == 0)
                {
                    // Incompatible memory type
                    continue;
                }

                const auto& props = memProperties.memoryTypes[i];
                if(((props.propertyFlags & requiredFlags) == requiredFlags)
                   && ((index == notFoundIndex)
                       || (props.propertyFlags & preferredFlags) == preferredFlags))
                {
                    if(memProperties.memoryHeaps[props.heapIndex].size
                       >= alignedSize(requirements.size, requirements.alignment))
                    {
                        index = i;
                    }
                }
            }
        }

        return index;
    }
} // namespace utils
} // namespace vkw