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

#include "DescriptorIndexing.hpp"

#include <cstdio>
#include <cstdlib>
#include <vkw/vkw.hpp>

int main(int /*argc*/, char** /*argv*/)
{
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    vkw::Instance instance{};
    VKW_CHECK_BOOL_RETURN_FALSE(instance.init(instanceLayers, {}));

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, nullptr);

    std::vector<VkPhysicalDevice> physicalDevices{physicalDeviceCount};
    vkEnumeratePhysicalDevices(instance.getHandle(), &physicalDeviceCount, physicalDevices.data());

    for(const auto physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        /// @note: At some point we would probably like to filter the devices to test. For now, use real GPUs.
        if((deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
           && (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU))
        {
            continue;
        }

        vkw::utils::Log::Info("TESTS", "Device name: %s", deviceProperties.deviceName);

        if(!launchDescriptorIndexingTestsTest(instance, physicalDevice))
        {
            vkw::utils::Log::Warning("TESTS", "Descriptor indexing test FAILED");
        }
    }

    return EXIT_SUCCESS;
}
