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

#pragma once

#include "vkWrappers/wrappers/Buffer.hpp"
#include "vkWrappers/wrappers/Common.hpp"
#include "vkWrappers/wrappers/ComputePipeline.hpp"
#include "vkWrappers/wrappers/Device.hpp"
#include "vkWrappers/wrappers/GraphicsPipeline.hpp"
#include "vkWrappers/wrappers/Image.hpp"
#include "vkWrappers/wrappers/Instance.hpp"
#include "vkWrappers/wrappers/RenderPass.hpp"
#include "vkWrappers/wrappers/Synchronization.hpp"
#include "vkWrappers/wrappers/utils.hpp"

#include <cstdio>
#include <cstdlib>

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

template <typename BufferType>
static inline VkBufferMemoryBarrier createBufferMemoryBarrier(
    const BufferType& buffer,
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

template <MemoryType memType>
static inline VkImageMemoryBarrier createImageMemoryBarrier(
    const Image<memType>& image,
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

class CommandBuffer
{
  public:
    CommandBuffer() {}
    CommandBuffer(Device& device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        VKW_CHECK_BOOL_THROW(this->init(device, commandPool, level), "Initializing command buffer");
    }

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer(CommandBuffer&& cp) { *this = std::move(cp); }

    CommandBuffer& operator=(const CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&& cp)
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

    bool init(Device& device, VkCommandPool commandPool, VkCommandBufferLevel level)
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

            VKW_INIT_CHECK_VK(device_->vk().vkAllocateCommandBuffers(
                device_->getHandle(), &allocateInfo, &commandBuffer_));

            initialized_ = true;
        }

        return true;
    }

    void clear()
    {
        if(cmdPool_ != VK_NULL_HANDLE)
        {
            if(recording_)
            {
                this->end();
            }
            device_->vk().vkFreeCommandBuffers(device_->getHandle(), cmdPool_, 1, &commandBuffer_);
        }

        device_ = nullptr;
        cmdPool_ = VK_NULL_HANDLE;
        commandBuffer_ = VK_NULL_HANDLE;

        recording_ = false;
        initialized_ = false;
    }

    bool isInitialized() const { return initialized_; }

    CommandBuffer& begin(VkCommandBufferUsageFlags usage = 0)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = usage;
        beginInfo.pInheritanceInfo = nullptr;

        VKW_CHECK_VK_THROW(
            device_->vk().vkBeginCommandBuffer(commandBuffer_, &beginInfo),
            "Starting recording commands");

        recording_ = true;
        return *this;
    }

    CommandBuffer& end()
    {
        VKW_CHECK_VK_THROW(
            device_->vk().vkEndCommandBuffer(commandBuffer_), "End recording commands");
        recording_ = false;
        return *this;
    }

    CommandBuffer& reset()
    {
        VKW_CHECK_VK_THROW(
            device_->vk().vkResetCommandBuffer(commandBuffer_, 0), "Restting command buffer");
        recording_ = false;
        return *this;
    }

    // ---------------------------------------------------------------------------

    template <typename SrcBufferType, typename DstBufferType, typename ArrayType>
    CommandBuffer& copyBuffer(SrcBufferType& src, DstBufferType& dst, ArrayType& regions)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdCopyBuffer(
            commandBuffer_,
            src.getHandle(),
            dst.getHandle(),
            static_cast<uint32_t>(regions.size()),
            reinterpret_cast<const VkBufferCopy*>(regions.data()));
        return *this;
    }

    template <typename SrcBufferType, typename DstBufferType>
    CommandBuffer& copyBuffer(SrcBufferType& src, DstBufferType& dst)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        VkBufferCopy copyData;
        copyData.dstOffset = 0;
        copyData.srcOffset = 0;
        copyData.size = src.sizeBytes(),

        device_->vk().vkCmdCopyBuffer(
            commandBuffer_, src.getHandle(), dst.getHandle(), 1, &copyData);
        return *this;
    }

    template <typename T, MemoryType memType>
    CommandBuffer& fillBuffer(
        Buffer<T, memType>& buffer, T val, const size_t offset, const size_t size)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdFillBuffer(
            commandBuffer_,
            buffer.getHandle(),
            static_cast<VkDeviceSize>(offset * sizeof(T)),
            static_cast<VkDeviceSize>(size),
            *((const uint32_t*) &val));
        return *this;
    }

    template <typename SrcBufferType, typename DstImageType>
    CommandBuffer& copyBufferToImage(
        SrcBufferType& buffer,
        DstImageType& image,
        VkImageLayout dstLayout,
        VkBufferImageCopy region)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdCopyBufferToImage(
            commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout, 1, &region);
        return *this;
    }

    template <typename SrcBufferType, typename DstImageType, typename ArrayType>
    CommandBuffer& copyBufferToImage(
        SrcBufferType& buffer, DstImageType& image, VkImageLayout dstLayout, ArrayType& regions)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdCopyBufferToImage(
            commandBuffer_,
            buffer.getHandle(),
            image.getHandle(),
            dstLayout,
            static_cast<uint32_t>(regions.size()),
            reinterpret_cast<const VkBufferImageCopy*>(regions.data()));
        return *this;
    }

    template <typename SrcImageType, typename DstBufferType>
    CommandBuffer& copyImageToBuffer(
        SrcImageType& image,
        VkImageLayout srcLayout,
        DstBufferType& buffer,
        VkBufferImageCopy region)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdCopyImageToBuffer(
            commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(), 1, &region);
        return *this;
    }

    template <typename SrcImageType, typename DstBufferType, typename ArrayType>
    CommandBuffer& copyImageToBuffer(
        SrcImageType& image, VkImageLayout srcLayout, DstBufferType& buffer, ArrayType regions)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdCopyImageToBuffer(
            commandBuffer_,
            image.getHandle(),
            srcLayout,
            buffer.getHandle(),
            static_cast<uint32_t>(regions.size()),
            reinterpret_cast<const VkBufferImageCopy*>(regions.data()));
        return *this;
    }

    template <typename SrcImageType, typename DstImageType>
    CommandBuffer& blitImage(
        const SrcImageType& src,
        const VkImageLayout srcLayout,
        DstImageType& dst,
        const VkImageLayout dstLayout,
        const VkImageBlit region,
        const VkFilter filter = VK_FILTER_LINEAR)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdBlitImage(
            commandBuffer_,
            src.getHandle(),
            srcLayout,
            dst.getHandle(),
            dstLayout,
            1,
            &region,
            filter);
        return *this;
    }
    template <typename SrcImageType, typename DstImageType>
    CommandBuffer& blitImage(
        const SrcImageType& src,
        const VkImageLayout srcLayout,
        DstImageType& dst,
        const VkImageLayout dstLayout,
        const std::vector<VkImageBlit>& regions,
        const VkFilter filter = VK_FILTER_LINEAR)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdBlitImage(
            commandBuffer_,
            src.getHandle(),
            srcLayout,
            dst.getHandle(),
            dstLayout,
            static_cast<uint32_t>(regions.size()),
            regions.data(),
            filter);
        return *this;
    }

    // -----------------------------------------------------------------------------

    template <typename... Args>
    CommandBuffer& memoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args&&... barriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkMemoryBarrier> barrierList({std::forward<Args>(barriers)...});
        device_->vk().vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            static_cast<uint32_t>(barrierList.size()),
            reinterpret_cast<const VkMemoryBarrier*>(barrierList.data()),
            0,
            nullptr,
            0,
            nullptr);
        return *this;
    }
    CommandBuffer& memoryBarrier(
        VkPipelineStageFlags srcFlags,
        VkPipelineStageFlags dstFlags,
        const VkMemoryBarrier& barrier)
    {
        return memoryBarriers(srcFlags, dstFlags, barrier);
    }

    template <typename... Args>
    CommandBuffer& bufferMemoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args&&... barriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkBufferMemoryBarrier> barrierList({std::forward<Args>(barriers)...});
        device_->vk().vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            0,
            nullptr,
            static_cast<uint32_t>(barrierList.size()),
            reinterpret_cast<const VkBufferMemoryBarrier*>(barrierList.data()),
            0,
            nullptr);
        return *this;
    }
    CommandBuffer& bufferMemoryBarrier(
        VkPipelineStageFlags srcFlags,
        VkPipelineStageFlags dstFlags,
        const VkBufferMemoryBarrier& barrier)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        return bufferMemoryBarriers(srcFlags, dstFlags, barrier);
    }

    template <typename... Args>
    CommandBuffer& imageMemoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args&&... barriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkImageMemoryBarrier> barrierList({std::forward<Args>(barriers)...});
        device_->vk().vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            0,
            nullptr,
            0,
            nullptr,
            static_cast<uint32_t>(barrierList.size()),
            reinterpret_cast<const VkImageMemoryBarrier*>(barrierList.data()));
        return *this;
    }
    CommandBuffer& imageMemoryBarrier(
        VkPipelineStageFlags srcFlags,
        VkPipelineStageFlags dstFlags,
        const VkImageMemoryBarrier& barrier)
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
    CommandBuffer& pipelineBarrier(
        const VkPipelineStageFlags srcFlags,
        const VkPipelineStageFlags dstFlags,
        const MemoryBarrierList& memoryBarriers,
        const BufferMemoryBarrierList& bufferMemoryBarriers,
        const ImageMemoryBarrierList& imageMemoryBarriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdPipelineBarrier(
            commandBuffer_,
            srcFlags,
            dstFlags,
            0,
            static_cast<uint32_t>(memoryBarriers.size()),
            reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.data()),
            static_cast<uint32_t>(bufferMemoryBarriers.size()),
            reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers.data()),
            static_cast<uint32_t>(imageMemoryBarriers.size()),
            reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers.data()));
    }

    // ---------------------------------------------------------------------------

    CommandBuffer& setEvent(const Event& event, const VkPipelineStageFlags flags)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdSetEvent(commandBuffer_, event.getHandle(), flags);
        return *this;
    }

    CommandBuffer& waitEvent(
        const Event& event,
        const VkPipelineStageFlags srcFlags,
        const VkPipelineStageFlags dstFlags,
        const std::vector<VkMemoryBarrier>& memoryBarriers,
        const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers,
        const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdWaitEvents(
            commandBuffer_,
            1,
            &event.getHandle(),
            srcFlags,
            dstFlags,
            static_cast<uint32_t>(memoryBarriers.size()),
            reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.data()),
            static_cast<uint32_t>(bufferMemoryBarriers.size()),
            reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers.data()),
            static_cast<uint32_t>(imageMemoryBarriers.size()),
            reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers.data()));
        return *this;
    }

    // ---------------------------------------------------------------------------

    CommandBuffer& bindComputePipeline(ComputePipeline& pipeline)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdBindPipeline(
            commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
        return *this;
    }

    CommandBuffer& bindComputeDescriptorSet(
        const PipelineLayout& pipelineLayout,
        const uint32_t firstSet,
        const DescriptorSet& descriptorSet)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        const auto descriptor = descriptorSet.getHandle();
        device_->vk().vkCmdBindDescriptorSets(
            commandBuffer_,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout.getHandle(),
            firstSet,
            1,
            &descriptor,
            0,
            nullptr);
        return *this;
    }

    CommandBuffer& bindComputeDescriptorSets(
        const PipelineLayout& pipelineLayout,
        const uint32_t firstSet,
        const std::vector<DescriptorSet>& descriptorSets)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        std::vector<VkDescriptorSet> descriptorList;
        for(size_t i = 0; i < descriptorSets.size(); ++i)
        {
            descriptorList.push_back(descriptorSets[i].getHandle());
        }
        device_->vk().vkCmdBindDescriptorSets(
            commandBuffer_,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            pipelineLayout.getHandle(),
            firstSet,
            static_cast<uint32_t>(descriptorList.size()),
            descriptorList.data(),
            0,
            nullptr);
        return *this;
    }

    template <typename T>
    CommandBuffer& pushConstants(
        PipelineLayout& pipelineLayout, VkShaderStageFlags flags, uint32_t offset, const T& values)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdPushConstants(
            commandBuffer_,
            pipelineLayout.getHandle(),
            flags,
            offset,
            static_cast<uint32_t>(sizeof(T)),
            reinterpret_cast<const void*>(&values));
        return *this;
    }

    CommandBuffer& dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdDispatch(commandBuffer_, x, y, z);
        return *this;
    }

    // ---------------------------------------------------------------------------

    CommandBuffer& beginRenderPass(
        RenderPass& renderPass,
        VkFramebuffer frameBuffer,
        const VkOffset2D& offset,
        const VkExtent2D& extent,
        const VkClearColorValue& clearColor)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.renderPass = renderPass.getHandle();
        renderPassInfo.framebuffer = frameBuffer;
        renderPassInfo.renderArea.offset = offset;
        renderPassInfo.renderArea.extent = extent;

        std::vector<VkClearValue> clearValues;
        clearValues.push_back(VkClearValue{clearColor});
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
        device_->vk().vkCmdBeginRenderPass(
            commandBuffer_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return *this;
    }

    CommandBuffer& nextSubpass(const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdNextSubpass(commandBuffer_, contents);
        return *this;
    }

    CommandBuffer& endRenderPass()
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdEndRenderPass(commandBuffer_);
        return *this;
    }

    CommandBuffer& beginRendering(
        ColorRenderTarget& colorAttachment,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = colorAttachment.imageView();
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
        attachmentInfo.resolveImageView = VK_NULL_HANDLE;
        attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentInfo.loadOp = colorAttachment.loadOp();
        attachmentInfo.storeOp = colorAttachment.storeOp();
        attachmentInfo.clearValue = colorAttachment.clearValue();

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = flags;
        renderingInfo.renderArea = renderArea;
        renderingInfo.viewMask = viewMask;
        renderingInfo.layerCount = layerCount;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &attachmentInfo;
        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;

        device_->vk().vkCmdBeginRendering(commandBuffer_, &renderingInfo);
        return *this;
    }

    CommandBuffer& beginRendering(
        const std::vector<ColorRenderTarget>& colorAttachments,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        std::vector<VkRenderingAttachmentInfo> attachmentInfos;
        attachmentInfos.resize(colorAttachments.size());
        for(const auto& colorAttachment : colorAttachments)
        {
            attachmentInfos.emplace_back();

            VkRenderingAttachmentInfo& attachmentInfo = attachmentInfos.back();
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = colorAttachment.imageView();
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
            attachmentInfo.resolveImageView = VK_NULL_HANDLE;
            attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentInfo.loadOp = colorAttachment.loadOp();
            attachmentInfo.storeOp = colorAttachment.storeOp();
            attachmentInfo.clearValue = colorAttachment.clearValue();
        }

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = flags;
        renderingInfo.renderArea = renderArea;
        renderingInfo.viewMask = viewMask;
        renderingInfo.layerCount = layerCount;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(attachmentInfos.size());
        renderingInfo.pColorAttachments = attachmentInfos.data();
        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;

        device_->vk().vkCmdBeginRendering(commandBuffer_, &renderingInfo);
        return *this;
    }

    CommandBuffer& beginRendering(
        ColorRenderTarget& colorAttachment,
        DepthStencilRenderTarget& depthStencilAttachment,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = colorAttachment.imageView();
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
        attachmentInfo.resolveImageView = VK_NULL_HANDLE;
        attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentInfo.loadOp = colorAttachment.loadOp();
        attachmentInfo.storeOp = colorAttachment.storeOp();
        attachmentInfo.clearValue = colorAttachment.clearValue();

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = depthStencilAttachment.imageView();
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
        attachmentInfo.resolveImageView = VK_NULL_HANDLE;
        attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentInfo.loadOp = depthStencilAttachment.loadOp();
        attachmentInfo.storeOp = depthStencilAttachment.storeOp();
        attachmentInfo.clearValue = depthStencilAttachment.clearValue();

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = flags;
        renderingInfo.renderArea = renderArea;
        renderingInfo.viewMask = viewMask;
        renderingInfo.layerCount = layerCount;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &attachmentInfo;
        renderingInfo.pDepthAttachment = &depthAttachmentInfo;
        renderingInfo.pStencilAttachment = nullptr;

        device_->vk().vkCmdBeginRendering(commandBuffer_, nullptr);
        return *this;
    }

    CommandBuffer& beginRendering(
        const std::vector<ColorRenderTarget>& colorAttachments,
        DepthStencilRenderTarget& depthStencilAttachment,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }

        std::vector<VkRenderingAttachmentInfo> attachmentInfos;
        attachmentInfos.resize(colorAttachments.size());
        for(const auto& colorAttachment : colorAttachments)
        {
            attachmentInfos.emplace_back();

            VkRenderingAttachmentInfo& attachmentInfo = attachmentInfos.back();
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = colorAttachment.imageView();
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
            attachmentInfo.resolveImageView = VK_NULL_HANDLE;
            attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentInfo.loadOp = colorAttachment.loadOp();
            attachmentInfo.storeOp = colorAttachment.storeOp();
            attachmentInfo.clearValue = colorAttachment.clearValue();
        }

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.pNext = nullptr;
        depthAttachmentInfo.imageView = depthStencilAttachment.imageView();
        depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.resolveMode = VK_RESOLVE_MODE_NONE;
        depthAttachmentInfo.resolveImageView = VK_NULL_HANDLE;
        depthAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentInfo.loadOp = depthStencilAttachment.loadOp();
        depthAttachmentInfo.storeOp = depthStencilAttachment.storeOp();
        depthAttachmentInfo.clearValue = depthStencilAttachment.clearValue();

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = flags;
        renderingInfo.renderArea = renderArea;
        renderingInfo.viewMask = viewMask;
        renderingInfo.layerCount = layerCount;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(attachmentInfos.size());
        renderingInfo.pColorAttachments = attachmentInfos.data();
        renderingInfo.pDepthAttachment = &depthAttachmentInfo;
        renderingInfo.pStencilAttachment = nullptr;

        device_->vk().vkCmdBeginRendering(commandBuffer_, nullptr);
        return *this;
    }

    CommandBuffer& endRendering()
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdEndRendering(commandBuffer_);
        return *this;
    }

    CommandBuffer& bindGraphicsPipeline(GraphicsPipeline& pipeline)
    {
        device_->vk().vkCmdBindPipeline(
            commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
        return *this;
    }

    CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout,
        const uint32_t firstSet,
        const DescriptorSet& descriptorSet)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        const auto descriptor = descriptorSet.getHandle();
        device_->vk().vkCmdBindDescriptorSets(
            commandBuffer_,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout.getHandle(),
            firstSet,
            1,
            &descriptor,
            0,
            nullptr);
        return *this;
    }

    CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout,
        const uint32_t firstSet,
        const std::vector<DescriptorSet>& descriptorSets)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        std::vector<VkDescriptorSet> descriptorList;
        for(size_t i = 0; i < descriptorSets.size(); ++i)
        {
            descriptorList.push_back(descriptorSets[i].getHandle());
        }
        device_->vk().vkCmdBindDescriptorSets(
            commandBuffer_,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout.getHandle(),
            firstSet,
            static_cast<uint32_t>(descriptorList.size()),
            descriptorList.data(),
            0,
            nullptr);
        return *this;
    }

    CommandBuffer& setViewport(
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
        device_->vk().vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);
        return *this;
    }

    CommandBuffer& setViewport(const VkViewport& viewport)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);
        return *this;
    }

    CommandBuffer& setScissor(const VkOffset2D& offset, const VkExtent2D& extent)
    {
        VkRect2D scissor{};
        scissor.offset = offset;
        scissor.extent = extent;

        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
        return *this;
    }

    CommandBuffer& setScissor(const VkRect2D& scissor)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
        return *this;
    }

    CommandBuffer& setCullMode(const VkCullModeFlags cullMode)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdSetCullMode(commandBuffer_, cullMode);
        return *this;
    }

    template <typename T, MemoryType memType>
    CommandBuffer& bindVertexBuffer(
        const uint32_t binding, const Buffer<T, memType>& buffer, const VkDeviceSize offset)
    {
        VkBuffer bufferHandle = buffer.getHandle();
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdBindVertexBuffers(commandBuffer_, binding, 1, &bufferHandle, &offset);
        return *this;
    }

    template <typename T, MemoryType memType>
    CommandBuffer& bindIndexBuffer(const Buffer<T, memType>& buffer, const VkIndexType indexType)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdBindIndexBuffer(commandBuffer_, buffer.getHandle(), 0, indexType);
        return *this;
    }

    CommandBuffer& draw(
        const uint32_t vertexCount,
        const uint32_t instanceCount,
        const uint32_t firstVertex,
        const uint32_t firstInstance)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdDraw(
            commandBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
        return *this;
    }

    CommandBuffer& drawIndexed(
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
        device_->vk().vkCmdDrawIndexed(
            commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        return *this;
    }

    CommandBuffer& drawMeshTasks(
        const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdDrawMeshTasksEXT(commandBuffer_, groupCountX, groupCountY, groupCountZ);
        return *this;
    }

    template <typename ParamsBufferType, typename CountBufferType, typename IndexType>
    CommandBuffer& drawMeshTasksIndirectCount(
        const ParamsBufferType& buffer,
        const VkDeviceSize offset,
        const CountBufferType& countBuffer,
        const VkDeviceSize countBufferOffset,
        const uint32_t maxDrawCount,
        const uint32_t stride)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdDrawMeshTasksIndirectCountEXT(
            commandBuffer_,
            buffer.getHandle(),
            offset,
            countBuffer.getHandle(),
            countBufferOffset,
            maxDrawCount,
            stride);
        return *this;
    }

    template <typename ParamsBufferType>
    CommandBuffer& drawMeshTasksIndirect(
        const ParamsBufferType& buffer,
        const VkDeviceSize offset,
        const uint32_t drawCount,
        const uint32_t stride)
    {
        if(!recording_)
        {
            throw std::runtime_error("Command buffer not in a recording state");
        }
        device_->vk().vkCmdDrawMeshTasksIndirectEXT(
            commandBuffer_, buffer.getHandle(), offset, drawCount, stride);
        return *this;
    }

    // ---------------------------------------------------------------------------

    VkCommandBuffer& getHandle() { return commandBuffer_; }
    const VkCommandBuffer& getHandle() const { return commandBuffer_; }

  private:
    Device* device_{nullptr};
    VkCommandPool cmdPool_{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};

    bool recording_{false};
    bool initialized_{false};
};
} // namespace vkw
