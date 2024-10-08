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

#include "Common.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// -----------------------------------------------------------------------------

int main(int, char **)
{
    const vkw::BufferPropertyFlags hostStagingFlags = {

        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    const vkw::BufferPropertyFlags deviceFlags
        = {VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT
               | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

    vkw::Instance instance(nullptr);
    vkw::Device device(instance);

    const size_t arraySize = 1024;
    auto v0 = randArray<float>(arraySize);
    std::vector<float> v1(arraySize);

    // Buffers creation
    vkw::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &b0 = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    auto &b1 = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    stagingMem.allocate();

    vkw::Memory deviceMem(device, deviceFlags.memoryFlags);
    auto &tmp = deviceMem.createBuffer<float>(deviceFlags.usage, arraySize);
    deviceMem.allocate();

    VkBufferMemoryBarrier barrier
        = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
           nullptr,
           VK_ACCESS_TRANSFER_WRITE_BIT,
           VK_ACCESS_TRANSFER_READ_BIT,
           VK_QUEUE_FAMILY_IGNORED,
           VK_QUEUE_FAMILY_IGNORED,
           tmp.getHandle(),
           0,
           VK_WHOLE_SIZE};

    std::vector<VkBufferCopy> c0 = {{0, 0, 1024 * sizeof(float)}};
    std::vector<VkBufferCopy> c1 = {{0, 0, 1024 * sizeof(float)}};

    vkw::CommandPool<vkw::QueueFamilyType::TRANSFER> cmdPool(device);
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(b0, tmp, c0)
        .bufferMemoryBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, barrier)
        .copyBuffer(tmp, b1, c1)
        .end();

    stagingMem.copyFromHost<float>(v0.data(), 0, v0.size());
    vkw::Queue<vkw::QueueFamilyType::TRANSFER> transferQueue(device);
    transferQueue.submit(cmdBuffer).waitIdle();
    stagingMem.copyFromDevice<float>(v1.data(), 0, v1.size());

    if(!compareArrays(v0, v1))
    {
        fprintf(stderr, "Error : test0 failed\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Test0 : success\n");

    fprintf(stdout, "Main : OK\n");
    return EXIT_SUCCESS;
}
