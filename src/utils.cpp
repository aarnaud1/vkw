/*
 * Copyright (c) 2026 Adrien ARNAUD
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

#include "vkw/detail/utils.hpp"

#include <fstream>
#include <string>
#include <volk.h>

namespace vkw
{
namespace utils
{
    VkShaderModule createShaderModule(
        const VolkDeviceTable& vk, const VkDevice device, const std::vector<char>& src)
    {
        VkShaderModule ret;

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = src.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(src.data());

        VKW_CHECK_VK_FAIL(
            vk.vkCreateShaderModule(device, &createInfo, nullptr, &ret), "Creating shader module");

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
        const VkPhysicalDevice physicalDevice, const VkMemoryPropertyFlags requiredFlags,
        const VkMemoryPropertyFlags preferredFlags, const VkMemoryPropertyFlags undesiredFlags,
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
            if((props.propertyFlags & undesiredFlags) != 0) { continue; }

            if(((props.propertyFlags & requiredFlags) == requiredFlags)
               && ((index == notFoundIndex) || (props.propertyFlags & preferredFlags) == preferredFlags))
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
                   && ((index == notFoundIndex) || (props.propertyFlags & preferredFlags) == preferredFlags))
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
