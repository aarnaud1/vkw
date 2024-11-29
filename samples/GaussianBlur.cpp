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
#include "ImgUtils.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// -----------------------------------------------------------------------------

// clang-format off
static const float gaussianKernel[] =
{
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    4.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    2.0f / 16.0f, 0.0f, 0.0f, 0.0f,
    1.0f / 16.0f, 0.0f, 0.0f, 0.0f
};

static const float laplacianKernel[] =
{
     0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     4.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.0f, 0.0f, 0.0f
};
// clang-format on

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

    int width;
    int height;
    uint8_t* imgData = utils::imgLoad("samples/data/img.png", &width, &height, 4);
    fprintf(stdout, "Image loaded : w = %d, h = %d\n", width, height);

    vkw::HostStagingBuffer<float> uboBuf(device, uniformDeviceFlags.usage, 36);
    vkw::HostStagingBuffer<float> imgBuffer(device, hostStagingFlags.usage, 4 * width * height);
    vkw::DeviceImage inImage(
        device,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        imgDeviceFlags.usage);
    vkw::DeviceImage outImage(
        device,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        imgDeviceFlags.usage);

    memcpy(uboBuf, gaussianKernel, 36 * sizeof(float));

    struct PushConstants
    {
        uint32_t width;
        uint32_t height;
    } pushConstants;
    pushConstants.width = static_cast<uint32_t>(width);
    pushConstants.height = static_cast<uint32_t>(height);

    vkw::PipelineLayout pipelineLayout(device, 1);
    pipelineLayout.getDescriptorSetLayout(0)
        .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0)
        .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1)
        .addUniformBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 2);

    uint32_t compPushConstantsOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants));

    pipelineLayout.create();

    vkw::ImageView inImageView(
        device,
        inImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    vkw::ImageView outImageView(
        device,
        outImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vkw::DescriptorPool descriptorPool(device, 16, 16);
    auto descriptorSet
        = descriptorPool.allocateDescriptorSet(pipelineLayout.getDescriptorSetLayout(0));
    descriptorSet.bindStorageImage(0, inImageView, VK_IMAGE_LAYOUT_GENERAL);
    descriptorSet.bindStorageImage(1, inImageView, VK_IMAGE_LAYOUT_GENERAL);
    descriptorSet.bindUniformBuffer(2, uboBuf);

    vkw::ComputePipeline pipeline(device, "output/spv/img_gaussian_comp.spv");
    pipeline.addSpec<uint32_t>(16).addSpec<uint32_t>(16);
    pipeline.createPipeline(pipelineLayout);

    vkw::CommandPool cmdPool(device, computeQueue);
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .imageMemoryBarriers(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createImageMemoryBarrier(
                inImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
            vkw::createImageMemoryBarrier(
                outImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL))
        .copyBufferToImage(
            imgBuffer,
            inImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            {0,
             0,
             0,
             {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
             {0, 0, 0},
             {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}})
        .imageMemoryBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            vkw::createImageMemoryBarrier(
                inImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL))
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSet(pipelineLayout, 0, descriptorSet)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantsOffset, pushConstants)
        .dispatch(vkw::utils::divUp(width, 16), vkw::utils::divUp(height, 16), 1)
        .imageMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vkw::createImageMemoryBarrier(
                outImage,
                VK_ACCESS_SHADER_WRITE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL))
        .copyImageToBuffer(
            outImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            imgBuffer,
            {0,
             0,
             0,
             {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
             {0, 0, 0},
             {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}})
        .end();

    // Execute
    std::vector<float> inData(width * height * 4);
    std::vector<float> outData(width * height * 4);
    for(int i = 0; i < width * height * 4; i++)
    {
        inData[i] = (float) imgData[i] / 255.0f;
    }

    vkw::Fence computeFence(device);

    imgBuffer.copyFromHost(inData.data(), imgBuffer.size());
    computeQueue.submit(cmdBuffer, computeFence);
    computeFence.wait();
    imgBuffer.copyToHost(outData.data(), imgBuffer.size());

    for(int i = 0; i < width * height * 4; i++)
    {
        imgData[i] = (unsigned char) (outData[i] * 255.0f);
    }

    utils::imgStorePNG("samples/data/output.png", imgData, width, height, 4);
    utils::imgFree(imgData);
    return EXIT_SUCCESS;
}
