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
    const std::vector<const char *> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<vkw::InstanceExtension> instanceExts = {vkw::DebugUtilsExt};
    vkw::Instance instance(instanceLayers, instanceExts);

    const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    vkw::Device device(instance, {}, {}, compatibleDeviceTypes);

    const size_t arraySize = 1025;
    auto X = randArray<float>(arraySize);
    auto Y = randArray<float>(arraySize);
    auto Z = randArray<float>(arraySize);

    vkw::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &xStagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    auto &yStagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    auto &zStagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, arraySize);
    stagingMem.allocate();

    vkw::Memory deviceMem(device, deviceFlags.memoryFlags);
    auto &dev_x = deviceMem.createBuffer<float>(deviceFlags.usage, arraySize);
    auto &dev_y = deviceMem.createBuffer<float>(deviceFlags.usage, arraySize);
    auto &dev_z = deviceMem.createBuffer<float>(deviceFlags.usage, arraySize);
    deviceMem.allocate();

    // Push constants for shader
    struct PushConstants
    {
        uint32_t maxSize;
    } pushConstants;
    pushConstants.maxSize = arraySize;

    // Configure shader module
    vkw::PipelineLayout pipelineLayout(device, 2);
    pipelineLayout.getDescriptorSetlayoutInfo(0)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0, 1)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1, 1);

    pipelineLayout.getDescriptorSetlayoutInfo(1).addStorageBufferBinding(
        VK_SHADER_STAGE_COMPUTE_BIT, 0, 1);

    uint32_t compPushConstantsOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants));

    pipelineLayout.create();

    vkw::DescriptorPool descriptorPool(device, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT);
    descriptorPool.bindStorageBuffer(0, 0, {dev_x.getHandle(), 0, VK_WHOLE_SIZE})
        .bindStorageBuffer(0, 1, {dev_y.getHandle(), 0, VK_WHOLE_SIZE})
        .bindStorageBuffer(1, 0, {dev_z.getHandle(), 0, VK_WHOLE_SIZE});

    vkw::ComputePipeline pipeline(device, "output/spv/array_add_comp.spv");
    pipeline.addSpec<uint32_t>(256);
    pipeline.createPipeline(pipelineLayout);

    // Commands recording
    vkw::CommandPool<vkw::QueueFamilyType::COMPUTE> cmdPool(device);
    std::array<VkBufferCopy, 1> c0 = {{0, 0, arraySize * sizeof(float)}};
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(xStagingBuf, dev_x, c0)
        .copyBuffer(yStagingBuf, dev_y, c0)
        .bufferMemoryBarriers(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            vkw::createBufferMemoryBarrier(
                dev_x, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT),
            vkw::createBufferMemoryBarrier(
                dev_y, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT))
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSets(pipelineLayout, descriptorPool)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantsOffset, pushConstants)
        .dispatch(vkw::utils::divUp(arraySize, 256), 1, 1)
        .bufferMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createBufferMemoryBarrier(
                dev_z, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT))
        .copyBuffer(dev_z, zStagingBuf, c0)
        .end();

    // Execute
    vkw::Queue<vkw::QueueFamilyType::COMPUTE> computeQueue(device);
    stagingMem.copyFromHost<float>(X.data(), xStagingBuf.getMemOffset(), X.size());
    stagingMem.copyFromHost<float>(Y.data(), yStagingBuf.getMemOffset(), Y.size());
    computeQueue.submit(cmdBuffer).waitIdle();
    stagingMem.copyFromDevice<float>(Z.data(), zStagingBuf.getMemOffset(), Z.size());

    for(size_t i = 0; i < arraySize; i++)
    {
        if(Z[i] != X[i] + Y[i])
        {
            fprintf(stderr, "ArrayAdd : fail\n");
            fprintf(stdout, "%f %f %f\n", Z[i], X[i], Y[i]);
            return EXIT_FAILURE;
        }
    }

    fprintf(stdout, "ArrayAdd : SUCCESS\n");
    return EXIT_SUCCESS;
}
