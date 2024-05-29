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

// -----------------------------------------------------------------------------

static void updateUBO(
    vk::Device &device, vk::Buffer<float> &uboBuf, float *data, const size_t size);

// -----------------------------------------------------------------------------

int main(int, char **)
{
    vk::Instance instance(nullptr);
    vk::Device device(instance);

    int width;
    int height;
    uint8_t *imgData = utils::imgLoad("main/data/img.png", &width, &height, 4);
    fprintf(stdout, "Image loaded : w = %d, h = %d\n", width, height);

    const uint32_t res = width * height;

    vk::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &stagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, 4 * res);
    stagingMem.allocate();

    vk::Memory uboMem(device, uniformDeviceFlags.memoryFlags);
    auto &uboBuf = uboMem.createBuffer<float>(uniformDeviceFlags.usage, 9 * 4);
    uboMem.allocate();

    vk::Memory imgMem(device, imgDeviceFlags.memoryFlags);
    auto &inImage = imgMem.createImage<vk::ImageFormat::RGBA, float>(
        VK_IMAGE_TYPE_2D,
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        imgDeviceFlags.usage);
    auto &outImage = imgMem.createImage<vk::ImageFormat::RGBA, float>(
        VK_IMAGE_TYPE_2D,
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        imgDeviceFlags.usage);
    imgMem.allocate();

    updateUBO(device, uboBuf, const_cast<float *>(gaussianKernel), 9 * 4);

    struct PushConstants
    {
        uint32_t width;
        uint32_t height;
    } pushConstants;
    pushConstants.width = static_cast<uint32_t>(width);
    pushConstants.height = static_cast<uint32_t>(height);

    vk::PipelineLayout pipelineLayout(device, 1);
    pipelineLayout.getDescriptorSetlayoutInfo(0)
        .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0, 1)
        .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1, 1)
        .addUniformBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 2, 1);

    uint32_t compPushConstantsOffset
        = pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PushConstants));

    pipelineLayout.create();

    vk::ImageView<vk::Image<vk::ImageFormat::RGBA, float>> inImageView(
        device,
        inImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    vk::ImageView<vk::Image<vk::ImageFormat::RGBA, float>> outImageView(
        device,
        outImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vk::DescriptorPool descriptorPool(device, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT);
    descriptorPool
        .bindStorageImage(0, 0, {VK_NULL_HANDLE, inImageView.getHandle(), VK_IMAGE_LAYOUT_GENERAL})
        .bindStorageImage(0, 1, {VK_NULL_HANDLE, outImageView.getHandle(), VK_IMAGE_LAYOUT_GENERAL})
        .bindUniformBuffer(0, 2, {uboBuf.getHandle(), 0, VK_WHOLE_SIZE});

    vk::ComputePipeline pipeline(device, "output/spv/img_gaussian_comp.spv");
    pipeline.addSpec<uint32_t>(16).addSpec<uint32_t>(16);
    pipeline.createPipeline(pipelineLayout);

    vk::CommandPool<vk::COMPUTE> cmdPool(device);
    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .imageMemoryBarriers(
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vk::createImageMemoryBarrier(
                inImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
            vk::createImageMemoryBarrier(
                outImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL))
        .copyBufferToImage<vk::RGBA, float>(
            stagingBuf,
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
            vk::createImageMemoryBarrier(
                inImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL))
        .bindComputePipeline(pipeline)
        .bindComputeDescriptorSets(pipelineLayout, descriptorPool)
        .pushConstants(
            pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantsOffset, &pushConstants)
        .dispatch(vk::divUp(width, 16), vk::divUp(height, 16), 1)
        .imageMemoryBarrier(
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            vk::createImageMemoryBarrier(
                outImage,
                VK_ACCESS_SHADER_WRITE_BIT,
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL))
        .copyImageToBuffer<vk::RGBA, float>(
            outImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            stagingBuf,
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

    auto computeQueue = vk::Queue<vk::QueueFamilyType::COMPUTE>(device);
    stagingMem.copyFromHost<float>(inData.data(), 0, inData.size());
    computeQueue.submit(cmdBuffer).waitIdle();
    stagingMem.copyFromDevice<float>(outData.data(), 0, outData.size());

    for(int i = 0; i < width * height * 4; i++)
    {
        imgData[i] = (unsigned char) (outData[i] * 255.0f);
    }

    utils::imgStorePNG("main/data/output.png", imgData, width, height, 4);
    utils::imgFree(imgData);
    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------

static void updateUBO(vk::Device &device, vk::Buffer<float> &uboBuf, float *data, const size_t size)
{
    vk::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto &stagingBuf = stagingMem.createBuffer<float>(hostStagingFlags.usage, size);
    stagingMem.allocate();
    stagingMem.copyFromHost<float>(data, stagingBuf.getOffset(), size);

    vk::CommandPool<vk::TRANSFER> cmdPool(device);
    std::array<VkBufferCopy, 1> c0 = {{0, 0, size * sizeof(float)}};

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(stagingBuf, uboBuf, c0)
        .end();

    vk::Queue<vk::QueueFamilyType::TRANSFER> transferQueue(device);
    transferQueue.submit(cmdBuffer).waitIdle();
}
