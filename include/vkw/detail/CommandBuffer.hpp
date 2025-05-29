/*
 * Copyright (c) 2025 Adrien ARNAUD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "vkw/detail/BottomLevelAccelerationStructure.hpp"
#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/ComputePipeline.hpp"
#include "vkw/detail/DescriptorSet.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/GraphicsPipeline.hpp"
#include "vkw/detail/Image.hpp"
#include "vkw/detail/Instance.hpp"
#include "vkw/detail/RenderPass.hpp"
#include "vkw/detail/RenderingAttachment.hpp"
#include "vkw/detail/Synchronization.hpp"
#include "vkw/detail/TopLevelAccelerationStructure.hpp"
#include "vkw/detail/utils.hpp"

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
static inline VkBufferMemoryBarrier createBufferMemoryBarrier(
    const VkBuffer buffer,
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
    ret.buffer = buffer;
    ret.offset = offset;
    ret.size = size;

    return ret;
}

template <typename ImageType>
static inline VkImageMemoryBarrier createImageMemoryBarrier(
    const ImageType& image,
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
static inline VkImageMemoryBarrier createImageMemoryBarrier(
    const VkImage image,
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
    ret.image = image;
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

class CommandBuffer
{
  public:
    CommandBuffer() {}
    CommandBuffer(const Device& device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        VKW_CHECK_BOOL_FAIL(this->init(device, commandPool, level), "Initializing command buffer");
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

    bool init(const Device& device, VkCommandPool commandPool, VkCommandBufferLevel level)
    {
        VKW_ASSERT(this->initialized() == false);

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

    bool initialized() const { return initialized_; }

    bool begin(VkCommandBufferUsageFlags usage = 0)
    {
        VKW_ASSERT(this->initialized());

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = usage;
        beginInfo.pInheritanceInfo = nullptr;

        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkBeginCommandBuffer(commandBuffer_, &beginInfo));
        recording_ = true;

        return true;
    }

    bool end()
    {
        VKW_ASSERT(this->initialized());
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkEndCommandBuffer(commandBuffer_));
        recording_ = false;

        return true;
    }

    bool reset()
    {
        VKW_ASSERT(this->initialized());
        VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkResetCommandBuffer(commandBuffer_, 0));
        recording_ = false;

        return false;
    }

    // ---------------------------------------------------------------------------------------------

    template <typename SrcBufferType, typename DstBufferType, typename ArrayType>
    CommandBuffer& copyBuffer(SrcBufferType& src, DstBufferType& dst, ArrayType& regions)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

        VkBufferCopy copyData;
        copyData.dstOffset = 0;
        copyData.srcOffset = 0;
        copyData.size = src.sizeBytes(),

        device_->vk().vkCmdCopyBuffer(
            commandBuffer_, src.getHandle(), dst.getHandle(), 1, &copyData);
        return *this;
    }

    template <typename BufferType, typename T>
    CommandBuffer& fillBuffer(BufferType& buffer, T val, const size_t offset, const size_t size)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

        device_->vk().vkCmdCopyBufferToImage(
            commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout, 1, &region);
        return *this;
    }

    template <typename SrcBufferType, typename DstImageType, typename ArrayType>
    CommandBuffer& copyBufferToImage(
        SrcBufferType& buffer, DstImageType& image, VkImageLayout dstLayout, ArrayType& regions)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

        device_->vk().vkCmdCopyImageToBuffer(
            commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(), 1, &region);
        return *this;
    }

    template <typename SrcImageType, typename DstBufferType, typename ArrayType>
    CommandBuffer& copyImageToBuffer(
        SrcImageType& image, VkImageLayout srcLayout, DstBufferType& buffer, ArrayType regions)
    {
        VKW_ASSERT(recording_);

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
        const DstImageType& dst,
        const VkImageLayout dstLayout,
        const VkImageBlit region,
        const VkFilter filter = VK_FILTER_LINEAR)
    {
        VKW_ASSERT(recording_);

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
    CommandBuffer& blitImage(
        const VkImage src,
        const VkImageLayout srcLayout,
        const VkImage dst,
        const VkImageLayout dstLayout,
        const VkImageBlit region,
        const VkFilter filter = VK_FILTER_LINEAR)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdBlitImage(
            commandBuffer_, src, srcLayout, dst, dstLayout, 1, &region, filter);
        return *this;
    }

    template <typename SrcImageType, typename DstImageType>
    CommandBuffer& blitImage(
        const SrcImageType& src,
        const VkImageLayout srcLayout,
        const DstImageType& dst,
        const VkImageLayout dstLayout,
        const std::vector<VkImageBlit>& regions,
        const VkFilter filter = VK_FILTER_LINEAR)
    {
        VKW_ASSERT(recording_);

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
    CommandBuffer& blitImage(
        const VkImage src,
        const VkImageLayout srcLayout,
        const VkImage dst,
        const VkImageLayout dstLayout,
        const std::vector<VkImageBlit>& regions,
        const VkFilter filter = VK_FILTER_LINEAR)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdBlitImage(
            commandBuffer_,
            src,
            srcLayout,
            dst,
            dstLayout,
            static_cast<uint32_t>(regions.size()),
            regions.data(),
            filter);
        return *this;
    }

    // ---------------------------------------------------------------------------------------------

    template <typename... Args>
    CommandBuffer& memoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args&&... barriers)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

        return bufferMemoryBarriers(srcFlags, dstFlags, barrier);
    }

    template <typename... Args>
    CommandBuffer& imageMemoryBarriers(
        VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags, Args&&... barriers)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

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

    // ---------------------------------------------------------------------------------------------

    CommandBuffer& setEvent(const Event& event, const VkPipelineStageFlags flags)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

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

    // ---------------------------------------------------------------------------------------------

    CommandBuffer& bindComputePipeline(ComputePipeline& pipeline)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdBindPipeline(
            commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
        return *this;
    }

    CommandBuffer& bindComputeDescriptorSet(
        const PipelineLayout& pipelineLayout,
        const uint32_t firstSet,
        const DescriptorSet& descriptorSet)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

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
        const PipelineLayout& pipelineLayout, const T& values, const ShaderStage stage)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdPushConstants(
            commandBuffer_,
            pipelineLayout.getHandle(),
            PipelineLayout::getVkShaderStage(stage),
            0,
            static_cast<uint32_t>(sizeof(T)),
            &values);

        return *this;
    }

    template <typename T, typename... Args>
    CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout,
        const ShaderStage stage,
        const T& values,
        Args&&... stages)
    {
        pushConstants(pipelineLayout, values, stage);
        return pushConstants(pipelineLayout, values, std::forward<Args>(stages)...);
    }

    CommandBuffer& dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdDispatch(commandBuffer_, x, y, z);
        return *this;
    }

    // ---------------------------------------------------------------------------------------------

    CommandBuffer& beginRenderPass(
        RenderPass& renderPass,
        VkFramebuffer frameBuffer,
        const VkOffset2D& offset,
        const VkExtent2D& extent,
        const VkClearColorValue& clearColor)
    {
        VKW_ASSERT(recording_);

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

        device_->vk().vkCmdBeginRenderPass(
            commandBuffer_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        return *this;
    }

    CommandBuffer& nextSubpass(const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdNextSubpass(commandBuffer_, contents);
        return *this;
    }

    CommandBuffer& endRenderPass()
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdEndRenderPass(commandBuffer_);
        return *this;
    }

    CommandBuffer& beginRendering(
        const RenderingAttachment& colorAttachment,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        VKW_ASSERT(recording_);

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = colorAttachment.attachment_;
        attachmentInfo.imageLayout = colorAttachment.imageLayout_;
        attachmentInfo.resolveMode = colorAttachment.resolveMode_;
        attachmentInfo.resolveImageView = colorAttachment.resolveAttachment_;
        attachmentInfo.resolveImageLayout = colorAttachment.resolveImageLayout_;
        attachmentInfo.loadOp = colorAttachment.loadOp_;
        attachmentInfo.storeOp = colorAttachment.storeOp_;
        attachmentInfo.clearValue = colorAttachment.clearValue_;

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
        const std::vector<RenderingAttachment>& colorAttachments,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        VKW_ASSERT(recording_);

        std::vector<VkRenderingAttachmentInfo> attachmentInfos;
        attachmentInfos.resize(colorAttachments.size());
        for(const auto& colorAttachment : colorAttachments)
        {
            attachmentInfos.emplace_back();

            VkRenderingAttachmentInfo& attachmentInfo = attachmentInfos.back();
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = colorAttachment.attachment_;
            attachmentInfo.imageLayout = colorAttachment.imageLayout_;
            attachmentInfo.resolveMode = colorAttachment.resolveMode_;
            attachmentInfo.resolveImageView = colorAttachment.resolveAttachment_;
            attachmentInfo.resolveImageLayout = colorAttachment.resolveImageLayout_;
            attachmentInfo.loadOp = colorAttachment.loadOp_;
            attachmentInfo.storeOp = colorAttachment.storeOp_;
            attachmentInfo.clearValue = colorAttachment.clearValue_;
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
        RenderingAttachment& colorAttachment,
        RenderingAttachment& depthStencilAttachment,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        VKW_ASSERT(recording_);

        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = colorAttachment.attachment_;
        attachmentInfo.imageLayout = colorAttachment.imageLayout_;
        attachmentInfo.resolveMode = colorAttachment.resolveMode_;
        attachmentInfo.resolveImageView = colorAttachment.resolveAttachment_;
        attachmentInfo.resolveImageLayout = colorAttachment.resolveImageLayout_;
        attachmentInfo.loadOp = colorAttachment.loadOp_;
        attachmentInfo.storeOp = colorAttachment.storeOp_;
        attachmentInfo.clearValue = colorAttachment.clearValue_;

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.pNext = nullptr;
        depthAttachmentInfo.imageView = depthStencilAttachment.imageView();
        depthAttachmentInfo.imageLayout = depthStencilAttachment.imageLayout_;
        depthAttachmentInfo.resolveMode = depthStencilAttachment.resolveMode_;
        depthAttachmentInfo.resolveImageView = depthStencilAttachment.resolveAttachment_;
        depthAttachmentInfo.resolveImageLayout = depthStencilAttachment.resolveImageLayout_;
        depthAttachmentInfo.loadOp = depthStencilAttachment.loadOp_;
        depthAttachmentInfo.storeOp = depthStencilAttachment.storeOp_;
        depthAttachmentInfo.clearValue = depthStencilAttachment.clearValue_;

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
        const std::vector<RenderingAttachment>& colorAttachments,
        RenderingAttachment& depthStencilAttachment,
        const VkRect2D renderArea,
        const uint32_t viewMask = 0,
        const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0)
    {
        VKW_ASSERT(recording_);

        std::vector<VkRenderingAttachmentInfo> attachmentInfos;
        attachmentInfos.resize(colorAttachments.size());
        for(const auto& colorAttachment : colorAttachments)
        {
            attachmentInfos.emplace_back();

            VkRenderingAttachmentInfo& attachmentInfo = attachmentInfos.back();
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.pNext = nullptr;
            attachmentInfo.imageView = colorAttachment.attachment_;
            attachmentInfo.imageLayout = colorAttachment.imageLayout_;
            attachmentInfo.resolveMode = colorAttachment.resolveMode_;
            attachmentInfo.resolveImageView = colorAttachment.resolveAttachment_;
            attachmentInfo.resolveImageLayout = colorAttachment.resolveImageLayout_;
            attachmentInfo.loadOp = colorAttachment.loadOp_;
            attachmentInfo.storeOp = colorAttachment.storeOp_;
            attachmentInfo.clearValue = colorAttachment.clearValue_;
        }

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.pNext = nullptr;
        depthAttachmentInfo.imageView = depthStencilAttachment.imageView();
        depthAttachmentInfo.imageLayout = depthStencilAttachment.imageLayout_;
        depthAttachmentInfo.resolveMode = depthStencilAttachment.resolveMode_;
        depthAttachmentInfo.resolveImageView = depthStencilAttachment.resolveAttachment_;
        depthAttachmentInfo.resolveImageLayout = depthStencilAttachment.resolveImageLayout_;
        depthAttachmentInfo.loadOp = depthStencilAttachment.loadOp_;
        depthAttachmentInfo.storeOp = depthStencilAttachment.storeOp_;
        depthAttachmentInfo.clearValue = depthStencilAttachment.clearValue_;

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
        VKW_ASSERT(recording_);

        device_->vk().vkCmdEndRendering(commandBuffer_);
        return *this;
    }

    CommandBuffer& bindGraphicsPipeline(GraphicsPipeline& pipeline)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdBindPipeline(
            commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
        return *this;
    }

    CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout,
        const uint32_t firstSet,
        const DescriptorSet& descriptorSet)
    {
        VKW_ASSERT(recording_);

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
        VKW_ASSERT(recording_);

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

    // ---------------------------------------------------------------------------------------------

    CommandBuffer& setViewport(
        const float offX,
        const float offY,
        const float width,
        const float height,
        const float minDepth = 0.0f,
        const float maxDepth = 1.0f)
    {
        VKW_ASSERT(recording_);

        VkViewport viewport{};
        viewport.x = offX;
        viewport.y = offY;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;

        device_->vk().vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);
        return *this;
    }

    CommandBuffer& setViewport(const VkViewport& viewport)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);
        return *this;
    }

    CommandBuffer& setViewport(const std::vector<VkViewport>& viewports, const uint32_t offset = 0)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetViewport(
            commandBuffer_, offset, static_cast<uint32_t>(viewports.size()), viewports.data());

        return *this;
    }

    CommandBuffer& setScissor(const VkOffset2D& offset, const VkExtent2D& extent)
    {
        VKW_ASSERT(recording_);

        VkRect2D scissor{};
        scissor.offset = offset;
        scissor.extent = extent;

        device_->vk().vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
        return *this;
    }

    CommandBuffer& setScissor(const VkRect2D& scissor)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
        return *this;
    }

    CommandBuffer& setScissor(const std::vector<VkRect2D>& scissors, const uint32_t offset = 0)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetScissor(
            commandBuffer_, offset, static_cast<uint32_t>(scissors.size()), scissors.data());

        return *this;
    }

    CommandBuffer& setLineWidth(const float lineWidth)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetLineWidth(commandBuffer_, lineWidth);

        return *this;
    }

    CommandBuffer& setDepthBias(
        const float depthBiasConstantFactor,
        const float depthBiasClamp,
        const float depthBiasSlopeFactor)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetDepthBias(
            commandBuffer_, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

        return *this;
    }

    CommandBuffer& setBlendConstants(const float r, const float g, const float b, const float a)
    {
        VKW_ASSERT(recording_);

        const float value[] = {r, g, b, a};
        device_->vk().vkCmdSetBlendConstants(commandBuffer_, value);

        return *this;
    }

    CommandBuffer& setStencilCompareMask(
        const VkStencilFaceFlags faceMask, const uint32_t compareMask)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetStencilCompareMask(commandBuffer_, faceMask, compareMask);

        return *this;
    }

    CommandBuffer& setStencilWriteMask(const VkStencilFaceFlags faceMask, const uint32_t writeMask)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetStencilWriteMask(commandBuffer_, faceMask, writeMask);

        return *this;
    }

    CommandBuffer& setStencilReference(const VkStencilFaceFlags faceMask, const uint32_t reference)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetStencilReference(commandBuffer_, faceMask, reference);

        return *this;
    }

    CommandBuffer& setCullMode(const VkCullModeFlags cullMode)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetCullMode(commandBuffer_, cullMode);

        return *this;
    }

    CommandBuffer& setFrontFace(const VkFrontFace frontFace)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetFrontFace(commandBuffer_, frontFace);

        return *this;
    }

    CommandBuffer& setPrimitiveTopology(const VkPrimitiveTopology topology)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetPrimitiveTopology(commandBuffer_, topology);

        return *this;
    }

    CommandBuffer& setViewportWithCount(const std::vector<VkViewport>& viewports)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetViewportWithCount(
            commandBuffer_, static_cast<uint32_t>(viewports.size()), viewports.data());

        return *this;
    }

    CommandBuffer& setScissorWithCount(const std::vector<VkRect2D>& scissors)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetScissorWithCount(
            commandBuffer_, static_cast<uint32_t>(scissors.size()), scissors.data());

        return *this;
    }

    CommandBuffer& setDepthTestEnable(const VkBool32 depthTestEnable)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetDepthTestEnable(commandBuffer_, depthTestEnable);

        return *this;
    }

    CommandBuffer& setDepthWriteEnable(const VkBool32 depthWriteEnable)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetDepthWriteEnable(commandBuffer_, depthWriteEnable);

        return *this;
    }

    CommandBuffer& setDepthCompareOp(const VkCompareOp compareOp)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetDepthCompareOp(commandBuffer_, compareOp);

        return *this;
    }

    CommandBuffer& setDepthBoundsTestEnable(const VkBool32 depthBoundsTestEnable)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetDepthBoundsTestEnable(commandBuffer_, depthBoundsTestEnable);

        return *this;
    }

    CommandBuffer& setStencilTestEnable(const VkBool32 stencilTestEnable)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetStencilTestEnable(commandBuffer_, stencilTestEnable);

        return *this;
    }

    CommandBuffer& setStencilOp(
        const VkStencilFaceFlags faceMask,
        const VkStencilOp failOp,
        const VkStencilOp passOp,
        const VkStencilOp depthFailOp,
        const VkCompareOp compareOp)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetStencilOp(
            commandBuffer_, faceMask, failOp, passOp, depthFailOp, compareOp);

        return *this;
    }

    CommandBuffer& setDepthBiasEnable(const VkBool32 depthBiasEnable)
    {
        VKW_ASSERT(recording_);

        device_->vk().vkCmdSetDepthBiasEnable(commandBuffer_, depthBiasEnable);

        return *this;
    }

    // ---------------------------------------------------------------------------------------------

    template <typename BufferType>
    CommandBuffer& bindVertexBuffer(
        const uint32_t binding, const BufferType& buffer, const VkDeviceSize offset)
    {
        VKW_ASSERT(recording_);
        VkBuffer bufferHandle = buffer.getHandle();
        device_->vk().vkCmdBindVertexBuffers(
            commandBuffer_, binding, 1, &bufferHandle, &offset * buffer.stride());
        return *this;
    }

    template <typename BufferType>
    CommandBuffer& bindVertexBuffer(
        const uint32_t binding,
        const BufferType& buffer,
        const VkDeviceSize offset,
        const VkDeviceSize size,
        const VkDeviceSize stride)
    {
        VKW_ASSERT(recording_);
        VkBuffer bufferHandle = buffer.getHandle();
        device_->vk().vkCmdBindVertexBuffers2(
            commandBuffer_, binding, 1, &bufferHandle, &offset * buffer.stride(), &size, &stride);
        return *this;
    }

    template <typename BufferType>
    CommandBuffer& bindIndexBuffer(const BufferType& buffer, const VkIndexType indexType)
    {
        VKW_ASSERT(recording_);
        device_->vk().vkCmdBindIndexBuffer(commandBuffer_, buffer.getHandle(), 0, indexType);
        return *this;
    }

    CommandBuffer& draw(
        const uint32_t vertexCount,
        const uint32_t instanceCount,
        const uint32_t firstVertex,
        const uint32_t firstInstance)
    {
        VKW_ASSERT(recording_);
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
        VKW_ASSERT(recording_);
        device_->vk().vkCmdDrawIndexed(
            commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        return *this;
    }

    CommandBuffer& drawMeshTasks(
        const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
    {
        VKW_ASSERT(recording_);
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
        VKW_ASSERT(recording_);
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
        VKW_ASSERT(recording_);
        device_->vk().vkCmdDrawMeshTasksIndirectEXT(
            commandBuffer_, buffer.getHandle(), offset, drawCount, stride);
        return *this;
    }

    // ---------------------------------------------------------------------------------------------

    template <typename BufferType>
    CommandBuffer& buildAccelerationStructure(
        const BottomLevelAccelerationStructure& blas,
        const BufferType& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {})
    {
        VKW_ASSERT(recording_);
        VKW_ASSERT(blas.buildOnHost_ == false);
        VKW_ASSERT(blas.geometryData_.size() == blas.buildRanges_.size());
        const auto* pBuildRanges = blas.buildRanges_.data();

        VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildInfo.pNext = nullptr;
        buildInfo.flags = buildFlags;
        buildInfo.type = blas.type();
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
        buildInfo.dstAccelerationStructure = blas.getHandle();
        buildInfo.geometryCount = static_cast<uint32_t>(blas.geometryData_.size());
        buildInfo.pGeometries = blas.geometryData_.data();
        buildInfo.ppGeometries = nullptr;
        buildInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress();
        device_->vk().vkCmdBuildAccelerationStructuresKHR(
            commandBuffer_, 1, &buildInfo, &pBuildRanges);

        return *this;
    }

    template <typename BufferType>
    CommandBuffer& buildAccelerationStructure(
        const TopLevelAccelerationStructure& tlas,
        const BufferType& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {})
    {
        VKW_ASSERT(recording_);
        VKW_ASSERT(tlas.buildOnHost_ == false);

        VkAccelerationStructureBuildRangeInfoKHR buildRange = {};
        buildRange.primitiveCount = static_cast<uint32_t>(tlas.instancesList_.size());

        const VkAccelerationStructureBuildRangeInfoKHR* pBuildRanges = &buildRange;

        VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildInfo.pNext = nullptr;
        buildInfo.flags = buildFlags;
        buildInfo.type = tlas.type();
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
        buildInfo.dstAccelerationStructure = tlas.getHandle();
        buildInfo.geometryCount = 1;
        buildInfo.pGeometries = &tlas.geometry_;
        buildInfo.ppGeometries = nullptr;
        buildInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress();
        device_->vk().vkCmdBuildAccelerationStructuresKHR(
            commandBuffer_, 1, &buildInfo, &pBuildRanges);

        return *this;
    }

    ///@todo Implement buildAccelerationStructures()
    ///@todo Implement buildAccelerationStructureIndirect
    ///@todo Implement buildAccelerationStructuresIndirect()
    ///@todo Implement copyAccelerationStructure()

    // ---------------------------------------------------------------------------------------------

    ///@todo Implement setRayTracingPipelineStackSize()
    ///@todo Implement traceRaysIndirect()
    ///@todo Implement traceRays()

    // ---------------------------------------------------------------------------------------------

    template <typename ComputeProgram>
    CommandBuffer& bindComputeProgram(const ComputeProgram& program, const uint32_t descriptorSetId)
    {
        VKW_ASSERT(recording_);

        this->bindComputePipeline(program.computePipeline_);
        this->bindComputeDescriptorSet(
            program.pipelineLayout_, 0, program.descriptorSets(descriptorSetId));

        return *this;
    }

    template <typename ComputeProgram, typename T>
    CommandBuffer& pushConstants(const ComputeProgram& program, const T& constants)
    {
        VKW_ASSERT(recording_);

        static_assert(
            std::is_same<T, typename ComputeProgram::constant_type>::value,
            "Push constant type mismatch");

        this->pushConstants(program.pipelineLayout_, constants, vkw::ShaderStage::Compute);

        return *this;
    }

    // ---------------------------------------------------------------------------------------------

    VkCommandBuffer getHandle() const { return commandBuffer_; }

  private:
    const Device* device_{nullptr};
    VkCommandPool cmdPool_{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};

    bool recording_{false};
    bool initialized_{false};
};
} // namespace vkw
