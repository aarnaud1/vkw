/*
 * Copyright (C) 2025 Adrien ARNAUD
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

bool testSaxpy(vkw::Device& device, uint32_t arraySize);

// -----------------------------------------------------------------------------

int main(int, char**)
{
    const int nTests = 16;

    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> instanceExts = {};
    vkw::Instance instance(instanceLayers, instanceExts);

    const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    vkw::Device device(instance, {}, {}, compatibleDeviceTypes);

    srand(static_cast<uint32_t>(time(NULL)));
    for(int i = 0; i < nTests; i++)
    {
        const uint32_t arraySize = uint32_t(rand() % 1000000);
        testSaxpy(device, arraySize);
    }
    return EXIT_SUCCESS;
}

bool testSaxpy(vkw::Device& device, uint32_t arraySize)
{
    const float alpha = randVal<float>();

    auto X = randArray<float>(arraySize);
    auto Y = randArray<float>(arraySize);

    vkw::DeviceBuffer<float> xDev(device, deviceFlags.usage, arraySize);
    vkw::DeviceBuffer<float> yDev(device, deviceFlags.usage, arraySize);

    struct
    {
        uint32_t maxSize;
        float alpha;
    } pushConstants;
    pushConstants.maxSize = arraySize;
    pushConstants.alpha = alpha;

    vkw::PipelineLayout pipelineLayout(device, 1);
    pipelineLayout.getDescriptorSetLayout(0)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0)
        .addStorageBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1);
    uint32_t compPushConstantOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants));
    pipelineLayout.create();

    vkw::DescriptorPool descriptorPool(device, 16, 16);
    auto descriptorSet
        = descriptorPool.allocateDescriptorSet(pipelineLayout.getDescriptorSetLayout(0));
    descriptorSet.bindStorageBuffer(0, xDev).bindStorageBuffer(1, yDev);

    vkw::ComputePipeline pipeline(device, "build/spv/array_saxpy_comp.spv");
    pipeline.addSpec<uint32_t>(256);
    pipeline.createPipeline(pipelineLayout);

    auto deviceQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_COMPUTE_BIT);
    if(deviceQueues.empty())
    {
        throw std::runtime_error("No available device queues");
    }
    vkw::Queue computeQueue = deviceQueues[0];

    vkw::CommandPool cmdPool(device, computeQueue);
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantOffset, pushConstants)
        .dispatch(vkw::utils::divUp(arraySize, 256), 1, 1)
        .bufferMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createBufferMemoryBarrier(
                yDev, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT))
        .end();

    // Launch work
    vkw::Fence computeFence(device);

    uploadData(device, X.data(), xDev);
    uploadData(device, Y.data(), yDev);
    computeQueue.submit(cmdBuffer, computeFence);
    computeFence.wait();

    std::vector<float> res(arraySize);
    downloadData(device, yDev, res.data());

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
