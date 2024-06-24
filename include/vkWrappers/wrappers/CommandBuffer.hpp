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

#pragma once

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/ComputePipeline.hpp"
#include "vkWrappers/wrappers/ComputeProgram.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/GraphicsPipeline.hpp"
#include "vkWrappers/wrappers/Image.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/QueueFamilies.hpp"
#include "vkWrappers/wrappers/RenderPass.hpp"
#include "vkWrappers/wrappers/Synchronization.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace vkw
{
static inline VkMemoryBarrier createMemoryBarrier(
    const VkAccessFlags srcMask, const VkAccessFlags dstMask)
{
    VkMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;

    return ret;
}

template <typename T>
static inline VkBufferMemoryBarrier createBufferMemoryBarrier(
    Buffer<T> &buffer,
    const VkAccessFlags srcMask,
    const VkAccessFlags dstMask,
    const VkDeviceSize offset = 0,
    const VkDeviceSize size = VK_WHOLE_SIZE)
{
    VkBufferMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.buffer = buffer.getHandle();
    ret.offset = offset;
    ret.size = size;

    return ret;
}

static inline VkImageMemoryBarrier createImageMemoryBarrier(
    Image &image,
    const VkAccessFlags srcMask,
    const VkAccessFlags dstMask,
    const VkImageLayout oldLayout,
    const VkImageLayout newLayout,
    const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    const uint32_t baseMipLevel = 0,
    const uint32_t levelCount = 1,
    const uint32_t baseArrayLayer = 0,
    const uint32_t layerCount = 1)
{
    VkImageMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.image = image.getHandle();
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

template <QueueFamilyType type>
class CommandBuffer
{
  public:
    CommandBuffer() {}
    CommandBuffer(Device &device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        this->init(device, commandPool, level);
    }

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&cp) { *this = std::move(cp); }

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&cp)
    {
        this->clear();
        std::swap(cp.device_, device_);
        std::swap(cp.commandBuffer_, commandBuffer_);
        std::swap(cp.cmdPool_, cmdPool_);

        std::swap(recording_, cp.recording_);
        std::swap(initialized_, cp.initialized_);
        return *this;
    }

    ~CommandBuffer() { this->clear(); }

    void init(Device &device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        if(!initialized_)
        {
            device_ = &device;
            cmdPool_ = commandPool;

            VkCommandBufferAllocateInfo allocateInfo;
            allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocateInfo.pNext = nullptr;
            allocateInfo.commandPool = cmdPool_;
            allocateInfo.level = level;
            allocateInfo.commandBufferCount = 1;

            CHECK_VK(
                vkAllocateCommandBuffers(device_->getHandle(), &allocateInfo, &commandBuffer_),
                "Allocating command buffer");

            initialized_ = true;
        }
    }

    void clear()
    {
        if(cmdPool_ != VK_NULL_HANDLE)
        {
            if(recording_)
            {
                this->end();
            }
            vkFreeCommandBuffers(device_->getHandle(), cmdPool_, 1, &commandBuffer_);
        }

        device_ = nullptr;
        cmdPool_ = VK_NULL_HANDLE;
        commandBuffer_ = VK_NULL_HANDLE;

        recording_ = false;
        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    CommandBuffer &begin(VkCommandBufferUsageFlags usage)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = usage;
        beginInfo.pInheritanceInfo = nullptr;

        CHECK_VK(vkBeginCommandBuffer(commandBuffer_, &beginInfo), "Starting recording commands");

        recording_ = true;
        return *this;
    }

    CommandBuffer &end()
    {
        CHECK_VK(vkEndCommandBuffer(commandBuffer_), "End recording commands");
        recording_ = false;
        return *this;
    }

    CommandBuffer &reset()
    {
        CHECK_VK(vkResetCommandBuffer(commandBuffer_, 0), "Restting command buffer");
        recording_ = false;
        return *this;
    }

    // ---------------------------------------------------------------------------

    template <typename SrcType, typename DstType, typename ArrayType>
    CommandBuffer &copyBuffer(Buffer<SrcType> &src, Buffer<DstType> &dst, ArrayType &regions)
    {
        static_assert(
            type == QueueFamilyType::GRAPHICS || type == QueueFamilyType::TRANSFER
                || type == QueueFamilyType::COMPUTE,
            "Error, queue must support graphics, compute or transfer operations");

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdCopyBuffer(
            commandBuffer_,
            src.getHandle(),
            dst.getHandle(),
            static_cast<uint32_t>(regions.size()),
            reinterpret_cast<const VkBufferCopy *>(regions.data()));
        return *this;
    }

    template <typename SrcType, typename DstType>
    CommandBuffer &copyBuffer(Buffer<SrcType> &src, Buffer<DstType> &dst)
    {
        static_assert(
            type == QueueFamilyType::GRAPHICS || type == QueueFamilyType::TRANSFER
                || type == QueueFamilyType::COMPUTE,
            "Error, queue must support graphics, compute or transfer operations");

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        VkBufferCopy copyData;
        copyData.dstOffset = 0;
        copyData.srcOffset = 0;
        copyData.size = src.getSizeBytes(),

        vkCmdCopyBuffer(commandBuffer_, src.getHandle(), dst.getHandle(), 1, &copyData);
        return *this;
    }

    template <typename T>
    CommandBuffer &fillBuffer(Buffer<T> &buffer, T val, const size_t offset, const size_t size)
    {
        static_assert(
            type == QueueFamilyType::GRAPHICS || type == QueueFamilyType::TRANSFER
                || type == QueueFamilyType::COMPUTE,
            "Error, queue must support graphics, compute or transfer operations");

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdFillBuffer(
            commandBuffer_,
            buffer.getHandle(),
            static_cast<VkDeviceSize>(offset * sizeof(T)),
            static_cast<VkDeviceSize>(size),
            *((const uint32_t *) &val));
        return *this;
    }

    template <typename T>
    CommandBuffer &copyBufferToImage(
        Buffer<T> &buffer, Image &image, VkImageLayout dstLayout, VkBufferImageCopy region)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdCopyBufferToImage(
            commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout, 1, &region);
        return *this;
    }

    template <typename T, typename ArrayType>
    CommandBuffer &copyBufferToImage(
        Buffer<T> &buffer, Image &image, VkImageLayout dstLayout, ArrayType &regions)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdCopyBufferToImage(
            commandBuffer_,
            buffer.getHandle(),
            image.getHandle(),
            dstLayout,
            static_cast<uint32_t>(regions.size()),
            reinterpret_cast<const VkBufferImageCopy *>(regions.data()));
        return *this;
    }

    template <typename T>
    CommandBuffer &copyImageToBuffer(
        Image &image, VkImageLayout srcLayout, Buffer<T> &buffer, VkBufferImageCopy region)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdCopyImageToBuffer(
            commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(), 1, &region);
        return *this;
    }

    template <typename T, typename ArrayType>
    CommandBuffer &copyImageToBuffer(
        Image &image, VkImageLayout srcLayout, Buffer<T> &buffer, ArrayType regions)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdCopyImageToBuffer(
            commandBuffer_,
            image.getHandle(),
            srcLayout,
            buffer.getHandle(),
            static_cast<uint32_t>(regions.size()),
            reinterpret_cast<const VkBufferImageCopy *>(regions.data()));
        return *this;
    }
    // -----------------------------------------------------------------------------

    template <typename... Args>
    CommandBuffer &memoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args &&...barriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkMemoryBarrier> barrierList({std::forward<Args>(barriers)...});
        vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            static_cast<uint32_t>(barrierList.size()),
            reinterpret_cast<const VkMemoryBarrier *>(barrierList.data()),
            0,
            nullptr,
            0,
            nullptr);
        return *this;
    }
    CommandBuffer &memoryBarrier(
        VkPipelineStageFlags srcFlags,
        VkPipelineStageFlags dstFlags,
        const VkMemoryBarrier &barrier)
    {
        return memoryBarriers(srcFlags, dstFlags, barrier);
    }

    template <typename... Args>
    CommandBuffer &bufferMemoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args &&...barriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkBufferMemoryBarrier> barrierList({std::forward<Args>(barriers)...});
        vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            0,
            nullptr,
            static_cast<uint32_t>(barrierList.size()),
            reinterpret_cast<const VkBufferMemoryBarrier *>(barrierList.data()),
            0,
            nullptr);
        return *this;
    }
    CommandBuffer &bufferMemoryBarrier(
        VkPipelineStageFlags srcFlags,
        VkPipelineStageFlags dstFlags,
        const VkBufferMemoryBarrier &barrier)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        return bufferMemoryBarriers(srcFlags, dstFlags, barrier);
    }

    template <typename... Args>
    CommandBuffer &imageMemoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args &&...barriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkImageMemoryBarrier> barrierList({std::forward<Args>(barriers)...});
        vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            0,
            nullptr,
            0,
            nullptr,
            static_cast<uint32_t>(barrierList.size()),
            reinterpret_cast<const VkImageMemoryBarrier *>(barrierList.data()));
        return *this;
    }
    CommandBuffer &imageMemoryBarrier(
        VkPipelineStageFlags srcFlags,
        VkPipelineStageFlags dstFlags,
        const VkImageMemoryBarrier &barrier)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        return imageMemoryBarriers(srcFlags, dstFlags, barrier);
    }

    template <
        typename MemoryBarrierList,
        typename BufferMemoryBarrierList,
        typename ImageMemoryBarrierList>
    CommandBuffer &pipelineBarrier(
        const VkPipelineStageFlags srcFlags,
        const VkPipelineStageFlags dstFlags,
        const MemoryBarrierList &memoryBarriers,
        const BufferMemoryBarrierList &bufferMemoryBarriers,
        const ImageMemoryBarrierList &imageMemoryBarriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            static_cast<uint32_t>(memoryBarriers.size()),
            reinterpret_cast<const VkMemoryBarrier *>(memoryBarriers.data()),
            static_cast<uint32_t>(bufferMemoryBarriers.size()),
            reinterpret_cast<const VkBufferMemoryBarrier *>(bufferMemoryBarriers.data()),
            static_cast<uint32_t>(imageMemoryBarriers.size()),
            reinterpret_cast<const VkImageMemoryBarrier *>(imageMemoryBarriers.data()));
    }

    // ---------------------------------------------------------------------------

    CommandBuffer &setEvent(const Event &event, const VkPipelineStageFlags flags)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdSetEvent(commandBuffer_, event.getHandle(), flags);
        return *this;
    }

    CommandBuffer &waitEvent(
        const Event &event,
        const VkPipelineStageFlags srcFlags,
        const VkPipelineStageFlags dstFlags,
        const std::vector<VkMemoryBarrier> &memoryBarriers,
        const std::vector<VkBufferMemoryBarrier> &bufferMemoryBarriers,
        const std::vector<VkImageMemoryBarrier> &imageMemoryBarriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdWaitEvents(
            commandBuffer_,
            1,
            &event.getHandle(),
            srcFlags,
            dstFlags,
            static_cast<uint32_t>(memoryBarriers.size()),
            reinterpret_cast<const VkMemoryBarrier *>(memoryBarriers.data()),
            static_cast<uint32_t>(bufferMemoryBarriers.size()),
            reinterpret_cast<const VkBufferMemoryBarrier *>(bufferMemoryBarriers.data()),
            static_cast<uint32_t>(imageMemoryBarriers.size()),
            reinterpret_cast<const VkImageMemoryBarrier *>(imageMemoryBarriers.data()));
        return *this;
    }

    // ---------------------------------------------------------------------------

    CommandBuffer &bindComputePipeline(ComputePipeline &pipeline)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
        return *this;
    }

    CommandBuffer &bindComputeDescriptorSets(
        PipelineLayout &pipelineLayout, DescriptorPool &descriptorPool)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        auto &descriptorSets = descriptorPool.getDescriptors();
        vkCmdBindDescriptorSets(
            commandBuffer_,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout.getHandle(),
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(),
            0,
            nullptr);
        return *this;
    }

    CommandBuffer &bindGraphicsDescriptorSets(
        PipelineLayout &pipelineLayout, DescriptorPool &descriptorPool)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        auto &descriptorSets = descriptorPool.getDescriptors();
        vkCmdBindDescriptorSets(
            commandBuffer_,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout.getHandle(),
            0,
            static_cast<uint32_t>(descriptorSets.size()),
            descriptorSets.data(),
            0,
            nullptr);
        return *this;
    }

    template <typename T>
    CommandBuffer &pushConstants(
        PipelineLayout &pipelineLayout, VkShaderStageFlags flags, uint32_t offset, const T &values)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdPushConstants(
            commandBuffer_,
            pipelineLayout.getHandle(),
            flags,
            offset,
            static_cast<uint32_t>(sizeof(T)),
            reinterpret_cast<const void *>(&values));
        return *this;
    }

    CommandBuffer &bindComputeProgram(ComputeProgram &program)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        return this->bindComputePipeline(program.computePipeline_)
            .bindComputeDescriptorSets(program.pipelineLayout_, program.descriptorPool_);
    }
    template <typename T>
    CommandBuffer &bindComputeProgram(ComputeProgram &program, T &pushConstants)
    {
        return this->bindComputeProgram(program).pushConstants(
            program.pipelineLayout_,
            VK_SHADER_STAGE_COMPUTE_BIT,
            program.pushConstantOffset_,
            pushConstants);
    }

    CommandBuffer &dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdDispatch(commandBuffer_, x, y, z);
        return *this;
    }

    // ---------------------------------------------------------------------------

    CommandBuffer &beginRenderPass(
        RenderPass &renderPass,
        VkFramebuffer frameBuffer,
        const VkOffset2D &offset,
        const VkExtent2D &extent,
        const glm::vec4 &clearColor)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.renderPass = renderPass.getHandle();
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = offset;
        renderPassInfo.renderArea.extent = extent;

        std::vector<VkClearValue> clearValues;
        clearValues.push_back(
            VkClearValue{{clearColor.r, clearColor.g, clearColor.b, clearColor.a}});
        if(renderPass.useDepth())
        {
            clearValues.push_back(VkClearValue{{1.0f, 0}});
        }
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdBeginRenderPass(commandBuffer_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return *this;
    }

    CommandBuffer &nextSubpass(const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdNextSubpass(commandBuffer_, contents);
        return *this;
    }

    CommandBuffer &bindGraphicsPipeline(GraphicsPipeline &pipeline)
    {
        vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
        return *this;
    }

    CommandBuffer &setViewport(
        const float offX,
        const float offY,
        const float width,
        const float height,
        const float minDepth = 0.0f,
        const float maxDepth = 1.0f)
    {
        VkViewport viewport{};
        viewport.x = offX;
        viewport.y = offY;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);
        return *this;
    }

    CommandBuffer &setScissor(const VkOffset2D &offset, const VkExtent2D &extent)
    {
        VkRect2D scissor{};
        scissor.offset = offset;
        scissor.extent = extent;

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
        return *this;
    }

    template <typename T>
    CommandBuffer &bindVertexBuffer(
        const uint32_t binding, const Buffer<T> &buffer, const VkDeviceSize offset)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdBindVertexBuffers(commandBuffer_, binding, 1, &(buffer.getHandle()), &offset);
        return *this;
    }

    template <typename T>
    CommandBuffer &bindIndexBuffer(const Buffer<T> &buffer, const VkIndexType indexType)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdBindIndexBuffer(commandBuffer_, buffer.getHandle(), 0, indexType);
        return *this;
    }

    CommandBuffer &draw(
        const uint32_t vertexCount,
        const uint32_t instanceCount,
        const uint32_t firstVertex,
        const uint32_t firstInstance)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdDraw(commandBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
        return *this;
    }

    CommandBuffer &drawIndexed(
        const uint32_t indexCount,
        const uint32_t instanceCount,
        const uint32_t firstIndex,
        const uint32_t vertexOffset,
        const uint32_t firstInstance)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdDrawIndexed(
            commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        return *this;
    }

    CommandBuffer &endRenderPass()
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        vkCmdEndRenderPass(commandBuffer_);
        return *this;
    }

    // ---------------------------------------------------------------------------

    VkCommandBuffer &getHandle() { return commandBuffer_; }
    const VkCommandBuffer &getHandle() const { return commandBuffer_; }

  private:
    Device *device_{nullptr};
    VkCommandPool cmdPool_{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};

    bool recording_{false};
    bool initialized_{false};
};
} // namespace vkw
