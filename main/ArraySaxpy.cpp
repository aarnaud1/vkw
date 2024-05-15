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

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>

#include "Common.hpp"

// -----------------------------------------------------------------------------

bool testSaxpy(vk::Device &device, size_t arraySize);

// -----------------------------------------------------------------------------

int main(int, char **)
{
    const int nTests = 16;

    vk::Instance instance(nullptr);
    vk::Device device(instance);

    srand(time(NULL));
    for(int i = 0; i < nTests; i++)
    {
        const size_t arraySize = size_t(rand() % 1000000);
        testSaxpy(device, arraySize);
    }
    return EXIT_SUCCESS;
}

bool testSaxpy(vk::Device &device, size_t arraySize)
{
    const float alpha = randVal<float>();

    auto X = randArray<float>(arraySize);
    auto Y = randArray<float>(arraySize);

    vk::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &xStagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    auto &yStagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    stagingMem.allocate();

    vk::Memory deviceMem(device, deviceFlags.memoryFlags);
    auto &xDev = deviceMem.createBuffer<float>(deviceFlags.usage, arraySize);
    auto &yDev = deviceMem.createBuffer<float>(deviceFlags.usage, arraySize);
    deviceMem.allocate();

    struct
    {
        uint32_t maxSize;
        float alpha;
    } pushConstants;
    pushConstants.maxSize = arraySize;
    pushConstants.alpha = alpha;

    vk::PipelineLayout pipelineLayout(device, 1);
    pipelineLayout.getDescriptorSetlayoutInfo(0)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0, 1)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);

    uint32_t compPushConstantOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants));

    pipelineLayout.create();

    vk::DescriptorPool descriptorPool(device, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT);
    descriptorPool.bindStorageBuffer(0, 0, {xDev.getHandle(), 0, VK_WHOLE_SIZE})
        .bindStorageBuffer(0, 1, {yDev.getHandle(), 0, VK_WHOLE_SIZE});

    vk::ComputePipeline pipeline(device, "output/spv/array_saxpy_comp.spv");
    pipeline.addSpec<uint32_t>(256);
    pipeline.createPipeline(pipelineLayout);

    vk::CommandPool<vk::QueueFamilyType::COMPUTE> cmdPool(device);
    std::array<VkBufferCopy, 1> c0 = {{0, 0, arraySize * sizeof(float)}};
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(xStagingBuf, xDev, c0)
        .copyBuffer(yStagingBuf, yDev, c0)
        .bufferMemoryBarriers(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            vk::createBufferMemoryBarrier(
                xDev, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT),
            vk::createBufferMemoryBarrier(
                yDev, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT))
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSets(pipelineLayout, descriptorPool)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantOffset, &pushConstants)
        .dispatch(vk::divUp(arraySize, 256), 1, 1)
        .bufferMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vk::createBufferMemoryBarrier(
                yDev, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT))
        .copyBuffer(yDev, yStagingBuf, c0)
        .end();

    // Launch work
    vk::Queue<vk::QueueFamilyType::COMPUTE> computeQueue(device);
    stagingMem.copyFromHost<float>(X.data(), xStagingBuf.getOffset(), arraySize);
    stagingMem.copyFromHost<float>(Y.data(), yStagingBuf.getOffset(), arraySize);
    computeQueue.submit(cmdBuffer).waitIdle();

    std::vector<float> res(arraySize);
    stagingMem.copyFromDevice<float>(res.data(), yStagingBuf.getOffset(), arraySize);

    for(size_t i = 0; i < arraySize; i++)
    {
        const float res0 = alpha * X[i] + Y[i];
        const float res1 = res[i];

        if(std::abs(res0 - res1) > 1E-5)
        {
            fprintf(stderr, "Array saxpy fail : %f %f\n", res0, res1);
            return false;
        }
    }

    fprintf(stdout, "Array saxpy : OK\n");
    return true;
}
