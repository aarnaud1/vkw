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

int main(int, char**)
{
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> instanceExts = {};
    vkw::Instance instance(instanceLayers, instanceExts);

    const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    vkw::Device device(instance, {}, {}, compatibleDeviceTypes);
    auto deviceQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_COMPUTE_BIT);
    if(deviceQueues.empty())
    {
        throw std::runtime_error("No available device queues");
    }
    vkw::Queue computeQueue = deviceQueues[0];

    const size_t arraySize = 1025;
    auto X = randArray<float>(arraySize);
    auto Y = randArray<float>(arraySize);
    auto Z = randArray<float>(arraySize);

    vkw::DeviceBuffer<float> xDevice(device, deviceFlags.usage, arraySize);
    vkw::DeviceBuffer<float> yDevice(device, deviceFlags.usage, arraySize);
    vkw::DeviceBuffer<float> zDevice(device, deviceFlags.usage, arraySize);

    // Push constants for shader
    struct PushConstants
    {
        uint32_t maxSize;
    } pushConstants;
    pushConstants.maxSize = arraySize;

    // Configure shader module
    const uint32_t setCount = 2;
    const uint32_t maxDescriptorCount = 16;

    vkw::DescriptorPool descriptorPool(device, setCount, maxDescriptorCount);
    vkw::PipelineLayout pipelineLayout(device, setCount);
    pipelineLayout.getDescriptorSetLayout(0)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    pipelineLayout.getDescriptorSetLayout(1).addStorageBufferBinding(
        VK_SHADER_STAGE_COMPUTE_BIT, 0);

    uint32_t compPushConstantsOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants));
    pipelineLayout.create();

    auto descriptorSet0
        = descriptorPool.allocateDescriptorSet(pipelineLayout.getDescriptorSetLayout(0));
    descriptorSet0.bindStorageBuffer(0, xDevice).bindStorageBuffer(1, yDevice);

    auto descriptorSet1
        = descriptorPool.allocateDescriptorSet(pipelineLayout.getDescriptorSetLayout(1));
    descriptorSet1.bindStorageBuffer(0, zDevice);

    vkw::ComputePipeline pipeline(device, "build/spv/array_add_comp.spv");
    pipeline.addSpec<uint32_t>(256);
    pipeline.createPipeline(pipelineLayout);

    // Commands recording
    vkw::CommandPool cmdPool(device, computeQueue);
    auto cmdBuffer = cmdPool.createCommandBuffer();

    vkw::Fence computeFence(device);
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet0)
        .bindComputeDescriptorSet(pipelineLayout, 1, descriptorSet1)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantsOffset, pushConstants)
        .dispatch(vkw::utils::divUp(arraySize, 256), 1, 1)
        .bufferMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createBufferMemoryBarrier(
                zDevice, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT))
        .end();

    // Execute
    uploadData(device, X.data(), xDevice);
    uploadData(device, Y.data(), yDevice);
    computeQueue.submit(cmdBuffer, computeFence);
    computeFence.wait();
    downloadData(device, zDevice, Z.data());

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
