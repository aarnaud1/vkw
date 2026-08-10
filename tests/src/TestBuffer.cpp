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

#include "TestBuffer.hpp"

#include "Utils.hpp"

#include <array>
#include <vector>
#include <vkw/vkw.hpp>

static const char* testName = "BufferTest";

static constexpr std::array<size_t, 4> bufferSizes = {1, 17, 4096, 1000000};
static constexpr std::array<VkBufferUsageFlags, 4> bufferUsages = {
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};

// -----------------------------------------------------------------------------------------------------------

static bool testBufferCreation(const vkw::Device& device);
static bool testBufferMove(const vkw::Device& device);
static bool testHostBufferAccess(const vkw::Device& device);
static bool testHostCoherentBufferAccess(const vkw::Device& device);
static bool testHostStagingBufferCopy(const vkw::Device& device);
static bool testDeviceUploadBufferCopy(const vkw::Device& device);
static bool testDeviceReadbackBufferCopy(const vkw::Device& device);
static bool testDeviceBufferUploadDownload(const vkw::Device& device);
static bool testHostDeviceBufferUploadDownload(const vkw::Device& device);
static bool testBufferPartialCopy(const vkw::Device& device);
static bool testBufferFill(const vkw::Device& device);
static bool testBufferDeviceAddress(const vkw::Device& device);

// -----------------------------------------------------------------------------------------------------------

bool launchBufferTests(const vkw::Instance& instance, const VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceBufferDeviceAddressFeatures availableBufferAddressFeatures = {};
    availableBufferAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    availableBufferAddressFeatures.pNext = nullptr;

    VkPhysicalDeviceFeatures2 availablePhysicalDeviceFeatures = {};
    availablePhysicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    availablePhysicalDeviceFeatures.pNext = &availableBufferAddressFeatures;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &availablePhysicalDeviceFeatures);

    const bool bufferAddressSupported = (availableBufferAddressFeatures.bufferDeviceAddress == VK_TRUE);

    vkw::Device device{};
    VKW_CHECK_BOOL_RETURN_FALSE(device.init(
        instance, physicalDevice, {}, {},
        bufferAddressSupported ? &availableBufferAddressFeatures : nullptr));

    uint32_t totalTests = 0;
    uint32_t failedTests = 0;

    vkw::utils::Log::Info(testName, "Checking buffer creation...");
    if(!testBufferCreation(device))
    {
        vkw::utils::Log::Warning(testName, "  Creation - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking buffer move semantics...");
    if(!testBufferMove(device))
    {
        vkw::utils::Log::Warning(testName, "  Move semantics - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking Host buffer access...");
    if(!testHostBufferAccess(device))
    {
        vkw::utils::Log::Warning(testName, "  Host buffer access - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking HostCoherent buffer access...");
    if(!testHostCoherentBufferAccess(device))
    {
        vkw::utils::Log::Warning(testName, "  HostCoherent buffer access - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking HostStaging buffer copy...");
    if(!testHostStagingBufferCopy(device))
    {
        vkw::utils::Log::Warning(testName, "  HostStaging buffer copy - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking DeviceUpload buffer copy...");
    if(!testDeviceUploadBufferCopy(device))
    {
        vkw::utils::Log::Warning(testName, "  DeviceUpload buffer copy - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking DeviceReadback buffer copy...");
    if(!testDeviceReadbackBufferCopy(device))
    {
        vkw::utils::Log::Warning(testName, "  DeviceReadback buffer copy - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking Device buffer upload/download...");
    if(!testDeviceBufferUploadDownload(device))
    {
        vkw::utils::Log::Warning(testName, "  Device buffer upload/download - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking HostDevice buffer upload/download...");
    if(!testHostDeviceBufferUploadDownload(device))
    {
        vkw::utils::Log::Warning(testName, "  HostDevice buffer upload/download - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking partial buffer copy...");
    if(!testBufferPartialCopy(device))
    {
        vkw::utils::Log::Warning(testName, "  Partial copy - FAILED");
        failedTests++;
    }
    totalTests++;

    vkw::utils::Log::Info(testName, "Checking fillBuffer...");
    if(!testBufferFill(device))
    {
        vkw::utils::Log::Warning(testName, "  fillBuffer - FAILED");
        failedTests++;
    }
    totalTests++;

    if(bufferAddressSupported == VK_TRUE)
    {
        vkw::utils::Log::Info(testName, "Checking buffer device address...");
        if(!testBufferDeviceAddress(device))
        {
            vkw::utils::Log::Warning(testName, "  Buffer device address - FAILED");
            failedTests++;
        }
        totalTests++;
    }

    vkw::utils::Log::Info(testName, "%u tests failed over %u", failedTests, totalTests);

    return failedTests == 0;
}

// -----------------------------------------------------------------------------------------------------------

template <vkw::MemoryType memType>
static bool testBufferCreationForMemType(const vkw::Device& device, const char* memTypeName)
{
    using Flags = vkw::MemoryFlags<memType>;

    bool ret = true;
    for(const auto size : bufferSizes)
    {
        for(const auto usage : bufferUsages)
        {
            vkw::Buffer<uint8_t, memType> buffer{device, size, usage};
            if(!buffer.initialized())
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] init failed (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
                continue;
            }

            if((buffer.size() != size) || (buffer.sizeBytes() != size) || (buffer.stride() != 1))
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] size mismatch (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
            }

            if((Flags::requiredFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && !buffer.deviceLocal())
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] expected deviceLocal (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
            }
            if((Flags::requiredFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) && !buffer.hostVisible())
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] expected hostVisible (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
            }
            if((Flags::requiredFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) && !buffer.hostCoherent())
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] expected hostCoherent (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
            }

            const auto fullInfo = buffer.getFullSizeInfo();
            if((fullInfo.buffer != buffer.getHandle()) || (fullInfo.offset != 0)
               || (fullInfo.range != buffer.sizeBytes()))
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] getFullSizeInfo mismatch (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
            }

            const size_t subOffset = size > 1 ? size / 2 : 0;
            const size_t subSize = size - subOffset;
            const auto subInfo = buffer.getDescriptorInfo(subOffset, subSize);
            if((subInfo.buffer != buffer.getHandle()) || (subInfo.offset != subOffset)
               || (subInfo.range != subSize))
            {
                vkw::utils::Log::Error(
                    testName, "  [%s] getDescriptorInfo mismatch (size=%zu, usage=%u)", memTypeName, size,
                    static_cast<uint32_t>(usage));
                ret = false;
            }
        }
    }
    return ret;
}

bool testBufferCreation(const vkw::Device& device)
{
    bool ret = true;
    ret &= testBufferCreationForMemType<vkw::MemoryType::Device>(device, "Device");
    ret &= testBufferCreationForMemType<vkw::MemoryType::Host>(device, "Host");
    ret &= testBufferCreationForMemType<vkw::MemoryType::HostCoherent>(device, "HostCoherent");
    ret &= testBufferCreationForMemType<vkw::MemoryType::HostDevice>(device, "HostDevice");
    ret &= testBufferCreationForMemType<vkw::MemoryType::HostStaging>(device, "HostStaging");
    ret &= testBufferCreationForMemType<vkw::MemoryType::DeviceUpload>(device, "DeviceUpload");
    ret &= testBufferCreationForMemType<vkw::MemoryType::DeviceReadback>(device, "DeviceReadback");
    return ret;
}

// -----------------------------------------------------------------------------------------------------------

template <vkw::MemoryType memType>
static bool testBufferMoveForMemType(const vkw::Device& device, const char* memTypeName)
{
    static constexpr size_t count = 128;

    vkw::Buffer<uint32_t, memType> buffer{
        device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    if(!buffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] init failed", memTypeName);
        return false;
    }

    const auto handle = buffer.getHandle();

    vkw::Buffer<uint32_t, memType> moved{std::move(buffer)};
    if(!moved.initialized() || (moved.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  [%s] move-construct failed", memTypeName);
        return false;
    }
    if(buffer.initialized() || (buffer.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  [%s] moved-from object not cleared", memTypeName);
        return false;
    }

    vkw::Buffer<uint32_t, memType> assigned{};
    assigned = std::move(moved);
    if(!assigned.initialized() || (assigned.getHandle() != handle))
    {
        vkw::utils::Log::Error(testName, "  [%s] move-assign failed", memTypeName);
        return false;
    }
    if(moved.initialized() || (moved.getHandle() != VK_NULL_HANDLE))
    {
        vkw::utils::Log::Error(testName, "  [%s] moved-from object not cleared after assign", memTypeName);
        return false;
    }

    return true;
}

bool testBufferMove(const vkw::Device& device)
{
    bool ret = true;
    ret &= testBufferMoveForMemType<vkw::MemoryType::Device>(device, "Device");
    ret &= testBufferMoveForMemType<vkw::MemoryType::Host>(device, "Host");
    ret &= testBufferMoveForMemType<vkw::MemoryType::HostCoherent>(device, "HostCoherent");
    ret &= testBufferMoveForMemType<vkw::MemoryType::HostDevice>(device, "HostDevice");
    ret &= testBufferMoveForMemType<vkw::MemoryType::HostStaging>(device, "HostStaging");
    ret &= testBufferMoveForMemType<vkw::MemoryType::DeviceUpload>(device, "DeviceUpload");
    ret &= testBufferMoveForMemType<vkw::MemoryType::DeviceReadback>(device, "DeviceReadback");
    return ret;
}

// -----------------------------------------------------------------------------------------------------------
// Host: supports both random access (behind an explicit map/unmap pair) and copyFromHost()/copyToHost().
// -----------------------------------------------------------------------------------------------------------

bool testHostBufferAccess(const vkw::Device& device)
{
    static constexpr size_t count = 4096;

    vkw::HostBuffer<float> buffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!buffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  Host buffer init failed");
        return false;
    }

    if(!buffer.mapMemory())
    {
        vkw::utils::Log::Error(testName, "  Host buffer mapMemory failed");
        return false;
    }

    TestUtils::fillPattern<float>(buffer.data(), count);
    if(!TestUtils::checkPattern<float>(buffer.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Host buffer data() write/read mismatch");
        return false;
    }

    std::vector<float> pattern2(count);
    TestUtils::fillPattern<float>(pattern2.data(), count, 2.0f);
    for(size_t i = 0; i < count; ++i)
    {
        buffer[i] = pattern2[i];
    }
    for(size_t i = 0; i < count; ++i)
    {
        if(buffer[i] != pattern2[i])
        {
            vkw::utils::Log::Error(testName, "  Host buffer operator[] mismatch at index %zu", i);
            return false;
        }
    }

    for(size_t i = 0; i < count; ++i)
    {
        if(buffer.begin()[i] != buffer[i])
        {
            vkw::utils::Log::Error(testName, "  Host buffer iterator mismatch at index %zu", i);
            return false;
        }
    }
    if(buffer.end() != buffer.begin() + count)
    {
        vkw::utils::Log::Error(testName, "  Host buffer end() mismatch");
        return false;
    }

    buffer.unmapMemory();

    std::vector<float> pattern3(count);
    TestUtils::fillPattern<float>(pattern3.data(), count, 3.0f);
    if(!buffer.copyFromHost(pattern3.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Host buffer copyFromHost failed");
        return false;
    }

    std::vector<float> readback(count);
    if(!buffer.copyToHost(readback.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Host buffer copyToHost failed");
        return false;
    }
    if(!TestUtils::compareData(pattern3.data(), readback.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Host buffer copyFromHost/copyToHost content mismatch");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------
// HostCoherent: permanently mapped, supports both random access and copyFromHost()/copyToHost().
// -----------------------------------------------------------------------------------------------------------

bool testHostCoherentBufferAccess(const vkw::Device& device)
{
    static constexpr size_t count = 4096;

    vkw::HostCoherentBuffer<float> buffer{device, count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!buffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  HostCoherent buffer init failed");
        return false;
    }

    TestUtils::fillPattern<float>(buffer.data(), count);
    if(!TestUtils::checkPattern<float>(buffer.data(), count))
    {
        vkw::utils::Log::Error(testName, "  HostCoherent buffer data() write/read mismatch");
        return false;
    }

    for(size_t i = 0; i < count; ++i)
    {
        if(buffer.begin()[i] != buffer[i])
        {
            vkw::utils::Log::Error(testName, "  HostCoherent buffer iterator mismatch at index %zu", i);
            return false;
        }
    }

    std::vector<float> pattern2(count);
    TestUtils::fillPattern<float>(pattern2.data(), count, 2.0f);
    if(!buffer.copyFromHost(pattern2.data(), count))
    {
        vkw::utils::Log::Error(testName, "  HostCoherent buffer copyFromHost failed");
        return false;
    }
    if(!TestUtils::checkPattern<float>(buffer.data(), count, 2.0f))
    {
        vkw::utils::Log::Error(testName, "  HostCoherent buffer copyFromHost content mismatch");
        return false;
    }

    std::vector<float> readback(count);
    if(!buffer.copyToHost(readback.data(), count))
    {
        vkw::utils::Log::Error(testName, "  HostCoherent buffer copyToHost failed");
        return false;
    }
    if(!TestUtils::compareData(pattern2.data(), readback.data(), count))
    {
        vkw::utils::Log::Error(testName, "  HostCoherent buffer copyToHost content mismatch");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------
// HostStaging / DeviceUpload / DeviceReadback: copy-only, no random access.
// -----------------------------------------------------------------------------------------------------------

template <vkw::MemoryType memType>
static bool testCopyOnlyBufferForMemType(
    const vkw::Device& device, const char* memTypeName, const size_t count,
    const VkBufferUsageFlags usage)
{
    std::vector<float> pattern(count);
    TestUtils::fillPattern<float>(pattern.data(), count);

    vkw::Buffer<float, memType> buffer{device, count, usage};
    if(!buffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] buffer init failed", memTypeName);
        return false;
    }

    if(!buffer.copyFromHost(pattern.data(), count))
    {
        vkw::utils::Log::Error(testName, "  [%s] copyFromHost failed", memTypeName);
        return false;
    }

    std::vector<float> readback(count);
    if(!buffer.copyToHost(readback.data(), count))
    {
        vkw::utils::Log::Error(testName, "  [%s] copyToHost failed", memTypeName);
        return false;
    }

    if(!TestUtils::compareData(pattern.data(), readback.data(), count))
    {
        vkw::utils::Log::Error(testName, "  [%s] content mismatch", memTypeName);
        return false;
    }

    return true;
}

bool testHostStagingBufferCopy(const vkw::Device& device)
{
    return testCopyOnlyBufferForMemType<vkw::MemoryType::HostStaging>(
        device, "HostStaging", 256, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

bool testDeviceUploadBufferCopy(const vkw::Device& device)
{
    return testCopyOnlyBufferForMemType<vkw::MemoryType::DeviceUpload>(
        device, "DeviceUpload", 1000000, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

bool testDeviceReadbackBufferCopy(const vkw::Device& device)
{
    return testCopyOnlyBufferForMemType<vkw::MemoryType::DeviceReadback>(
        device, "DeviceReadback", 1000000,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

// -----------------------------------------------------------------------------------------------------------
// Device / HostDevice: opaque GPU resources, accessed exclusively through Vulkan commands.
// -----------------------------------------------------------------------------------------------------------

template <vkw::MemoryType memType>
static bool testDeviceBufferUploadDownloadForMemType(const vkw::Device& device, const char* memTypeName)
{
    static constexpr size_t count = 4096;

    std::vector<float> pattern(count);
    TestUtils::fillPattern<float>(pattern.data(), count);

    vkw::Buffer<float, memType> src{
        device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    vkw::Buffer<float, memType> dst{
        device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    if(!src.initialized() || !dst.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] buffer init failed", memTypeName);
        return false;
    }

    if(!TestUtils::uploadToDeviceBuffer(device, pattern.data(), src, count))
    {
        vkw::utils::Log::Error(testName, "  [%s] upload failed", memTypeName);
        return false;
    }

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    if(!cmdPool.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] command pool init failed", memTypeName);
        return false;
    }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] command buffer init failed", memTypeName);
        return false;
    }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.copyBuffer(src, dst);
    cmdBuffer.bufferMemoryBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createBufferMemoryBarrier(dst, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized())
    {
        vkw::utils::Log::Error(testName, "  [%s] fence init failed", memTypeName);
        return false;
    }
    if(transferQueue.submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        vkw::utils::Log::Error(testName, "  [%s] submit failed", memTypeName);
        return false;
    }
    if(!fence.wait())
    {
        vkw::utils::Log::Error(testName, "  [%s] fence wait failed", memTypeName);
        return false;
    }

    std::vector<float> result(count);
    if(!TestUtils::downloadFromDeviceBuffer(device, dst, result.data(), count))
    {
        vkw::utils::Log::Error(testName, "  [%s] download failed", memTypeName);
        return false;
    }

    if(!TestUtils::compareData(pattern.data(), result.data(), count))
    {
        vkw::utils::Log::Error(testName, "  [%s] copyBuffer content mismatch", memTypeName);
        return false;
    }

    return true;
}

bool testDeviceBufferUploadDownload(const vkw::Device& device)
{
    return testDeviceBufferUploadDownloadForMemType<vkw::MemoryType::Device>(device, "Device");
}

bool testHostDeviceBufferUploadDownload(const vkw::Device& device)
{
    return testDeviceBufferUploadDownloadForMemType<vkw::MemoryType::HostDevice>(device, "HostDevice");
}

// -----------------------------------------------------------------------------------------------------------

bool testBufferPartialCopy(const vkw::Device& device)
{
    static constexpr size_t count = 1024;
    static constexpr size_t copyOffset = 256;
    static constexpr size_t copyCount = 512;

    std::vector<float> pattern(count);
    TestUtils::fillPattern<float>(pattern.data(), count);

    vkw::DeviceBuffer<float> src{
        device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    vkw::DeviceBuffer<float> dst{
        device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    if(!src.initialized() || !dst.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy buffer init failed");
        return false;
    }

    if(!TestUtils::uploadToDeviceBuffer(device, pattern.data(), src, count))
    {
        vkw::utils::Log::Error(testName, "  Partial copy upload failed");
        return false;
    }

    std::vector<float> zeros(count, 0.0f);
    if(!TestUtils::uploadToDeviceBuffer(device, zeros.data(), dst, count))
    {
        vkw::utils::Log::Error(testName, "  Partial copy dst clear failed");
        return false;
    }

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    if(!cmdPool.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy command pool init failed");
        return false;
    }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy command buffer init failed");
        return false;
    }

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = copyOffset * sizeof(float);
    copyRegion.dstOffset = copyOffset * sizeof(float);
    copyRegion.size = copyCount * sizeof(float);

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.copyBuffer(src, dst, std::span<VkBufferCopy>(&copyRegion, 1));
    cmdBuffer.bufferMemoryBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createBufferMemoryBarrier(dst, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized())
    {
        vkw::utils::Log::Error(testName, "  Partial copy fence init failed");
        return false;
    }
    if(transferQueue.submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        vkw::utils::Log::Error(testName, "  Partial copy submit failed");
        return false;
    }
    if(!fence.wait())
    {
        vkw::utils::Log::Error(testName, "  Partial copy fence wait failed");
        return false;
    }

    std::vector<float> result(count);
    if(!TestUtils::downloadFromDeviceBuffer(device, dst, result.data(), count))
    {
        vkw::utils::Log::Error(testName, "  Partial copy download failed");
        return false;
    }

    for(size_t i = 0; i < count; ++i)
    {
        const bool inCopiedRange = (i >= copyOffset) && (i < copyOffset + copyCount);
        const float expected = inCopiedRange ? pattern[i] : 0.0f;
        if(result[i] != expected)
        {
            vkw::utils::Log::Error(testName, "  Partial copy content mismatch at index %zu", i);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testBufferFill(const vkw::Device& device)
{
    static constexpr size_t count = 1024;
    static constexpr size_t fillOffset = 128;
    static constexpr size_t fillCount = 256;
    static constexpr uint32_t fillValue = 0xABCDABCDu;

    vkw::DeviceBuffer<uint32_t> buffer{
        device, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    if(!buffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  fillBuffer target init failed");
        return false;
    }

    std::vector<uint32_t> zeros(count, 0);
    if(!TestUtils::uploadToDeviceBuffer(device, zeros.data(), buffer, count))
    {
        vkw::utils::Log::Error(testName, "  fillBuffer target clear failed");
        return false;
    }

    auto transferQueue = device.getQueues(vkw::QueueUsageBits::Transfer)[0];

    vkw::CommandPool cmdPool{device, transferQueue};
    if(!cmdPool.initialized())
    {
        vkw::utils::Log::Error(testName, "  fillBuffer command pool init failed");
        return false;
    }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    if(!cmdBuffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  fillBuffer command buffer init failed");
        return false;
    }

    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.fillBuffer(buffer, fillValue, fillOffset, fillCount * sizeof(uint32_t));
    cmdBuffer.bufferMemoryBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createBufferMemoryBarrier(buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT));
    cmdBuffer.end();

    vkw::Fence fence{device};
    if(!fence.initialized())
    {
        vkw::utils::Log::Error(testName, "  fillBuffer fence init failed");
        return false;
    }
    if(transferQueue.submit(cmdBuffer, fence) != VK_SUCCESS)
    {
        vkw::utils::Log::Error(testName, "  fillBuffer submit failed");
        return false;
    }
    if(!fence.wait())
    {
        vkw::utils::Log::Error(testName, "  fillBuffer fence wait failed");
        return false;
    }

    std::vector<uint32_t> result(count);
    if(!TestUtils::downloadFromDeviceBuffer(device, buffer, result.data(), count))
    {
        vkw::utils::Log::Error(testName, "  fillBuffer download failed");
        return false;
    }

    for(size_t i = 0; i < count; ++i)
    {
        const bool inFilledRange = (i >= fillOffset) && (i < fillOffset + fillCount);
        const uint32_t expected = inFilledRange ? fillValue : 0;
        if(result[i] != expected)
        {
            vkw::utils::Log::Error(testName, "  fillBuffer content mismatch at index %zu", i);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------

bool testBufferDeviceAddress(const vkw::Device& device)
{
    vkw::DeviceBuffer<float, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT> buffer{
        device, 256, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT};
    if(!buffer.initialized())
    {
        vkw::utils::Log::Error(testName, "  Buffer device address: buffer init failed");
        return false;
    }

    if(buffer.deviceAddress() == 0)
    {
        vkw::utils::Log::Error(testName, "  Buffer device address: got null address");
        return false;
    }

    return true;
}