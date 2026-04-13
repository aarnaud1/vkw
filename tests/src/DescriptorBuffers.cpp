<<<<<<< HEAD
=======
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

>>>>>>> fb9c23d (Add cladd DescriptorBuffer)
#include "Utils.hpp"

#include <memory>
#include <vkw/vkw.hpp>

static const char* testName = "DescriptorBuffersTest";

// -----------------------------------------------------------------------------------------------------------

static bool testStorageBufferDescriptorBuffer(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

static bool testStorageBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize);

// -----------------------------------------------------------------------------------------------------------

static const uint32_t fillStorageBuffersDescriptorBufferIndexingComp[] = {
#include "spv/FillStorageBuffersDescriptorBufferIndexing.comp.spv"
};

// -----------------------------------------------------------------------------------------------------------

bool launchDescriptorBuffersTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extProperties;
    extProperties.resize(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extProperties.data());

    bool supportsDescriptorBuffers = false;
    for(const auto& extProperty : extProperties)
    {
        if(strcmp(extProperty.extensionName, VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME) == 0)
        {
            supportsDescriptorBuffers = true;
        }
    }

    if(!supportsDescriptorBuffers)
    {
        vkw::utils::Log::Info(
            testName, "Descriptor buffers not available for this physical device, skipping");
        return true;
    }

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
    bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceAddressFeatures.pNext = nullptr;
    bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures availabeDescriptorIndexingFeatures = {};
    availabeDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    availabeDescriptorIndexingFeatures.pNext = &bufferDeviceAddressFeatures;

    VkPhysicalDeviceFeatures2 availablePhysicalDeviceFeatures = {};
    availablePhysicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    availablePhysicalDeviceFeatures.pNext = &availabeDescriptorIndexingFeatures;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &availablePhysicalDeviceFeatures);

    bool supportsIndexing
        = (availabeDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount != VK_FALSE)
          && (availabeDescriptorIndexingFeatures.descriptorBindingPartiallyBound != VK_FALSE);

    std::vector<const char*> deviceExtensions = {};
    if(supportsDescriptorBuffers) { deviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME); }

    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(
        device.init(instance, physicalDevice, deviceExtensions, {}, &availabeDescriptorIndexingFeatures));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    vkw::utils::Log::Info(testName, "Checking storage buffer descriptor buffer...");
    for(size_t i = 1; i <= 16; ++i)
    {
        if(!testStorageBufferDescriptorBuffer(device, i, 1024))
        {
            vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
            failedTests++;
        }
        totalTests++;
    }

    if(supportsIndexing)
    {
        vkw::utils::Log::Info(testName, "Checking storage buffer descriptor buffer with indexing...");
        for(size_t i = 1; i <= 16; ++i)
        {
            if(!testStorageBufferDescriptorBufferIndexing(device, i, 1024))
            {
                vkw::utils::Log::Warning(testName, "  Descriptor count %zu - FAILED", i);
                failedTests++;
            }
            totalTests++;
        }
    }

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testStorageBufferDescriptorBuffer(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    return true;
}

bool testStorageBufferDescriptorBufferIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t bufferSize)
{
    const size_t alignment = device.getDescriptorBufferProperties().descriptorBufferOffsetAlignment;
    const size_t storageBufferDescSize = device.getDescriptorBufferProperties().storageBufferDescriptorSize;
    const auto descBufferSize = vkw::utils::alignedSize(storageBufferDescSize, alignment);

    vkw::DescriptorBuffer<vkw::MemoryType::HostStaging> descriptorBuffer{};
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorBuffer.init(device, descBufferSize));

    return true;
}