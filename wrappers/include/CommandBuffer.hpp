/*
 * Copyright (C) 2022 Adrien ARNAUD
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

#pragma once

#include <cstdlib>
#include <cstdio>

#include <vulkan/vulkan.h>

#include "utils.hpp"
#include "Instance.hpp"
#include "QueueFamilies.hpp"
#include "Device.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "ComputePipeline.hpp"

namespace vk
{
template <QueueFamilyType type>
class CommandBuffer
{
public:
  CommandBuffer(
      Device &device, VkCommandPool commandPool, VkCommandBufferLevel level)
      : device_(device), cmdPool_(commandPool)
  {
    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandPool = cmdPool_;
    allocateInfo.level = level;
    allocateInfo.commandBufferCount = 1;

    CHECK_VK(
        vkAllocateCommandBuffers(
            device_.getHandle(), &allocateInfo, &commandBuffer_),
        "Allocating command buffer");
  }

  ~CommandBuffer()
  {
    vkFreeCommandBuffers(device_.getHandle(), cmdPool_, 1, &commandBuffer_);
  }

  CommandBuffer &begin(VkCommandBufferUsageFlags usage)
  {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = usage;
    beginInfo.pInheritanceInfo = nullptr;

    CHECK_VK(
        vkBeginCommandBuffer(commandBuffer_, &beginInfo),
        "Starting recording commands");
    return *this;
  }

  CommandBuffer &end()
  {
    CHECK_VK(vkEndCommandBuffer(commandBuffer_), "End recording commands");
    return *this;
  }

  CommandBuffer &reset()
  {
    CHECK_VK(
        vkResetCommandBuffer(commandBuffer_, 0), "Restting command buffer");
    return *this;
  }

  // ---------------------------------------------------------------------------

  template <typename SrcType, typename DstType, typename ArrayType>
  CommandBuffer &
  copyBuffer(Buffer<SrcType> &src, Buffer<DstType> &dst, ArrayType &regions)
  {
    static_assert(
        type == QueueFamilyType::GRAPHICS || type == QueueFamilyType::TRANSFER
            || type == QueueFamilyType::COMPUTE,
        "Error, queue must support graphics, compute or transfer operations");
    vkCmdCopyBuffer(
        commandBuffer_, src.getHandle(), dst.getHandle(),
        static_cast<uint32_t>(regions.size()),
        reinterpret_cast<const VkBufferCopy *>(regions.data()));
    return *this;
  }

  template <typename T>
  CommandBuffer &
  fillBuffer(Buffer<T> &buffer, T val, const size_t offset, const size_t size)
  {
    static_assert(
        type == QueueFamilyType::GRAPHICS || type == QueueFamilyType::TRANSFER
            || type == QueueFamilyType::COMPUTE,
        "Error, queue must support graphics, compute or transfer operations");
    vkCmdFillBuffer(
        commandBuffer_, buffer.getHandle(),
        static_cast<VkDeviceSize>(offset * sizeof(T)),
        static_cast<VkDeviceSize>(size), *((const uint32_t *) &val));
    return *this;
  }

  template <ImageFormat format, typename T>
  CommandBuffer &copyBufferToImage(
      Buffer<T> &buffer, Image<format, T> &image, VkImageLayout dstLayout,
      VkBufferImageCopy region)
  {
    vkCmdCopyBufferToImage(
        commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout, 1,
        &region);
    return *this;
  }

  // template <ImageFormat format, typename T, typename ArrayType>
  // CommandBuffer &copyBufferToImage(
  //     Buffer<T> &buffer, Image<format, T> &image, VkImageLayout dstLayout,
  //     ArrayType &regions)
  // {
  //   vkCmdCopyBufferToImage(
  //       commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout,
  //       static_cast<uint32_t>(regions.size()),
  //       reinterpret_cast<const VkBufferImageCopy *>(regions.data()));
  //   return *this;
  // }

  template <ImageFormat format, typename T>
  CommandBuffer &copyImageToBuffer(
      Image<format, T> &image, VkImageLayout srcLayout, Buffer<T> &buffer,
      VkBufferImageCopy region)
  {
    vkCmdCopyImageToBuffer(
        commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(), 1,
        &region);
    return *this;
  }

  template <ImageFormat format, typename T, typename ArrayType>
  CommandBuffer &copyImageToBuffer(
      Image<format, T> &image, VkImageLayout srcLayout, Buffer<T> &buffer,
      ArrayType regions)
  {
    vkCmdCopyImageToBuffer(
        commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(),
        static_cast<uint32_t>(regions.size()),
        reinterpret_cast<const VkBufferImageCopy *>(regions.data()));
    return *this;
  }
  // -----------------------------------------------------------------------------

  template <typename ArrayType>
  CommandBuffer &memoryBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      ArrayType &barriers)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0,
        static_cast<uint32_t>(barriers.size()),
        reinterpret_cast<const VkMemoryBarrier *>(barriers.data()), 0, nullptr,
        0, nullptr);
    return *this;
  }

  CommandBuffer &memoryBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      VkMemoryBarrier &barrier)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 1,
        reinterpret_cast<const VkMemoryBarrier *>(&barrier), 0, nullptr, 0,
        nullptr);
    return *this;
  }

  template <typename ArrayType>
  CommandBuffer &bufferMemoryBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      ArrayType &barriers)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr,
        static_cast<uint32_t>(barriers.size()),
        reinterpret_cast<const VkBufferMemoryBarrier *>(barriers.data()), 0,
        nullptr);
    return *this;
  }

  CommandBuffer &bufferMemoryBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      VkBufferMemoryBarrier &barrier)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, 1,
        reinterpret_cast<const VkBufferMemoryBarrier *>(&barrier), 0, nullptr);
    return *this;
  }

  template <typename ArrayType>
  CommandBuffer &imageMemoryBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      ArrayType &barriers)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr,
        static_cast<uint32_t>(barriers.size()),
        reinterpret_cast<const VkImageMemoryBarrier *>(barriers.data()));
    return *this;
  }

  CommandBuffer &imageMemoryBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      VkImageMemoryBarrier &barrier)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1,
        reinterpret_cast<const VkImageMemoryBarrier *>(&barrier));
    return *this;
  }

  template <
      typename MemoryBarrierList, typename BufferMemoryBarrierList,
      typename ImageMemoryBarrierList>
  CommandBuffer &pipelineBarrier(
      VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags,
      MemoryBarrierList &memoryBarriers,
      BufferMemoryBarrierList &bufferMemoryBarriers,
      ImageMemoryBarrierList &imageMemoryBarriers)
  {
    vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0,
        static_cast<uint32_t>(memoryBarriers.size()),
        reinterpret_cast<const VkMemoryBarrier *>(memoryBarriers.data()),
        static_cast<uint32_t>(bufferMemoryBarriers.size()),
        reinterpret_cast<const VkBufferMemoryBarrier *>(
            bufferMemoryBarriers.data()),
        static_cast<uint32_t>(imageMemoryBarriers.size()),
        reinterpret_cast<const VkImageMemoryBarrier *>(
            imageMemoryBarriers.data()));
  }

  // ---------------------------------------------------------------------------

  CommandBuffer &bindComputePipeline(ComputePipeline &pipeline)
  {
    vkCmdBindPipeline(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
    return *this;
  }

  CommandBuffer &bindComputeDescriptorSets(
      PipelineLayout &pipelineLayout, DescriptorPool &descriptorPool)
  {
    auto &descriptorSets = descriptorPool.getDescriptors();
    vkCmdBindDescriptorSets(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE,
        pipelineLayout.getHandle(), 0,
        static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0,
        nullptr);
    return *this;
  }

  template <typename T>
  CommandBuffer &pushConstants(
      PipelineLayout &pipelineLayout, VkShaderStageFlags flags, uint32_t offset,
      T *values)
  {
    vkCmdPushConstants(
        commandBuffer_, pipelineLayout.getHandle(), flags, offset,
        static_cast<uint32_t>(sizeof(T)), reinterpret_cast<void *>(values));
    return *this;
  }

  CommandBuffer &dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1)
  {
    vkCmdDispatch(commandBuffer_, x, y, z);
    return *this;
  }

  // ---------------------------------------------------------------------------

  VkCommandBuffer &getHandle() { return commandBuffer_; }

private:
  Device &device_;
  VkCommandPool cmdPool_;
  VkCommandBuffer commandBuffer_;
};
} // namespace vk
