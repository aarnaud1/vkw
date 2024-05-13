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

#include "vkWrappers/Instance.hpp"
#include "vkWrappers/Device.hpp"
#include "vkWrappers/Memory.hpp"
#include "vkWrappers/Buffer.hpp"
#include "vkWrappers/QueueFamilies.hpp"
#include "vkWrappers/CommandPool.hpp"
#include "vkWrappers/PipelineLayout.hpp"
#include "vkWrappers/DescriptorPool.hpp"
#include "vkWrappers/ComputePipeline.hpp"

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

  uint32_t compPushConstantOffset =
      pipelineLayout.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(pushConstants));

  pipelineLayout.create();

  vk::DescriptorPool descriptorPool(device, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT);
  descriptorPool.bindStorageBuffer(0, 0, {xDev.getHandle(), 0, VK_WHOLE_SIZE})
      .bindStorageBuffer(0, 1, {yDev.getHandle(), 0, VK_WHOLE_SIZE});

  vk::ComputePipeline pipeline(device, "spv/array_saxpy.spv");
  pipeline.addSpec<uint32_t>(256);
  pipeline.createPipeline(pipelineLayout);

  // Barriers
  std::vector<VkBufferMemoryBarrier> transferBarriers = {
      {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
       VK_ACCESS_SHADER_READ_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
       xDev.getHandle(), 0, VK_WHOLE_SIZE},
      {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT,
       VK_ACCESS_SHADER_READ_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
       yDev.getHandle(), 0, VK_WHOLE_SIZE}};

  std::vector<VkBufferMemoryBarrier> computeBarriers = {
      {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT,
       VK_ACCESS_TRANSFER_READ_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
       yDev.getHandle(), 0, VK_WHOLE_SIZE},
  };

  vk::CommandPool<vk::QueueFamilyType::COMPUTE> cmdPool(device);
  std::array<VkBufferCopy, 1> c0 = {{0, 0, arraySize * sizeof(float)}};
  auto cmdBuffer = cmdPool.createCommandBuffer();
  cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
      .copyBuffer(xStagingBuf, xDev, c0)
      .copyBuffer(yStagingBuf, yDev, c0)
      .bufferMemoryBarrier(
          VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, transferBarriers)
      .bindComputePipeline(pipeline)
      .bindComputeDescriptorSets(pipelineLayout, descriptorPool)
      .pushConstants(
          pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, compPushConstantOffset, &pushConstants)
      .dispatch(vk::divUp(arraySize, 256), 1, 1)
      .bufferMemoryBarrier(
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, computeBarriers)
      .copyBuffer(yDev, yStagingBuf, c0)
      .end();

  // Launch work
  stagingMem.copyFromHost<float>(X.data(), xStagingBuf.getOffset(), arraySize);
  stagingMem.copyFromHost<float>(Y.data(), yStagingBuf.getOffset(), arraySize);
  device.getQueue<vk::QueueFamilyType::COMPUTE>().submit(cmdBuffer.getHandle()).waitIdle();

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
