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

#include "vkw/detail/CommandBuffer.hpp"

#include "vkw/detail/utils.hpp"

namespace vkw
{
CommandBuffer::CommandBuffer(const Device& device, VkCommandPool commandPool, VkCommandBufferLevel level)
{
    VKW_CHECK_BOOL_FAIL(this->init(device, commandPool, level), "Initializing command buffer");
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& cp)
{
    this->clear();
    std::swap(cp.device_, device_);
    std::swap(cp.commandBuffer_, commandBuffer_);
    std::swap(cp.cmdPool_, cmdPool_);

    std::swap(recording_, cp.recording_);
    std::swap(initialized_, cp.initialized_);
    return *this;
}

bool CommandBuffer::init(const Device& device, VkCommandPool commandPool, VkCommandBufferLevel level)
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

    VKW_INIT_CHECK_VK(
        device_->vk().vkAllocateCommandBuffers(device_->getHandle(), &allocateInfo, &commandBuffer_));

    initialized_ = true;

    return true;
}

void CommandBuffer::clear()
{
    if(cmdPool_ != VK_NULL_HANDLE)
    {
        if(recording_) { this->end(); }
        device_->vk().vkFreeCommandBuffers(device_->getHandle(), cmdPool_, 1, &commandBuffer_);
    }

    device_ = nullptr;
    cmdPool_ = VK_NULL_HANDLE;
    commandBuffer_ = VK_NULL_HANDLE;

    recording_ = false;
    initialized_ = false;
}

bool CommandBuffer::begin(VkCommandBufferUsageFlags usage)
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

bool CommandBuffer::end()
{
    VKW_ASSERT(this->initialized());
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkEndCommandBuffer(commandBuffer_));
    recording_ = false;

    return true;
}

bool CommandBuffer::reset()
{
    VKW_ASSERT(this->initialized());
    VKW_CHECK_VK_RETURN_FALSE(device_->vk().vkResetCommandBuffer(commandBuffer_, 0));
    recording_ = false;

    return false;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::copyBuffer(
    const BaseBuffer& src, const BaseBuffer& dst, const std::span<VkBufferCopy>& regions)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdCopyBuffer(
        commandBuffer_, src.getHandle(), dst.getHandle(), static_cast<uint32_t>(regions.size()),
        reinterpret_cast<const VkBufferCopy*>(regions.data()));
    return *this;
}

CommandBuffer& CommandBuffer::copyBuffer(const BaseBuffer& src, const BaseBuffer& dst)
{
    VKW_ASSERT(recording_);

    VkBufferCopy copyData;
    copyData.dstOffset = 0;
    copyData.srcOffset = 0;
    copyData.size = src.sizeBytes(),

    device_->vk().vkCmdCopyBuffer(commandBuffer_, src.getHandle(), dst.getHandle(), 1, &copyData);
    return *this;
}

CommandBuffer& CommandBuffer::fillBuffer(
    const BaseBuffer& buffer, const uint32_t val, const size_t offset, const size_t size)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdFillBuffer(
        commandBuffer_, buffer.getHandle(), static_cast<VkDeviceSize>(offset * sizeof(uint32_t)),
        static_cast<VkDeviceSize>(size), val);
    return *this;
}

CommandBuffer& CommandBuffer::copyBufferToImage(
    const BaseBuffer& buffer, const BaseImage& image, const VkImageLayout dstLayout,
    const VkBufferImageCopy& region)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdCopyBufferToImage(
        commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout, 1, &region);
    return *this;
}

CommandBuffer& CommandBuffer::copyBufferToImage(
    const BaseBuffer& buffer, const BaseImage& image, VkImageLayout dstLayout,
    const std::span<VkBufferImageCopy>& regions)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdCopyBufferToImage(
        commandBuffer_, buffer.getHandle(), image.getHandle(), dstLayout,
        static_cast<uint32_t>(regions.size()), reinterpret_cast<const VkBufferImageCopy*>(regions.data()));
    return *this;
}

CommandBuffer& CommandBuffer::copyImageToBuffer(
    const BaseImage& image, VkImageLayout srcLayout, const BaseBuffer& buffer,
    const VkBufferImageCopy& region)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdCopyImageToBuffer(
        commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(), 1, &region);
    return *this;
}

CommandBuffer& CommandBuffer::copyImageToBuffer(
    const BaseImage& image, VkImageLayout srcLayout, const BaseBuffer& buffer,
    const std::span<VkBufferImageCopy>& regions)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdCopyImageToBuffer(
        commandBuffer_, image.getHandle(), srcLayout, buffer.getHandle(),
        static_cast<uint32_t>(regions.size()), reinterpret_cast<const VkBufferImageCopy*>(regions.data()));
    return *this;
}

CommandBuffer& CommandBuffer::blitImage(
    const BaseImage& src, const VkImageLayout srcLayout, const BaseImage& dst, const VkImageLayout dstLayout,
    const VkImageBlit region, const VkFilter filter)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBlitImage(
        commandBuffer_, src.getHandle(), srcLayout, dst.getHandle(), dstLayout, 1, &region, filter);
    return *this;
}

CommandBuffer& CommandBuffer::blitImage(
    const VkImage src, const VkImageLayout srcLayout, const VkImage dst, const VkImageLayout dstLayout,
    const VkImageBlit region, const VkFilter filter)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBlitImage(commandBuffer_, src, srcLayout, dst, dstLayout, 1, &region, filter);
    return *this;
}

CommandBuffer& CommandBuffer::blitImage(
    const BaseImage& src, const VkImageLayout srcLayout, const BaseImage& dst, const VkImageLayout dstLayout,
    const std::span<VkImageBlit>& regions, const VkFilter filter)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBlitImage(
        commandBuffer_, src.getHandle(), srcLayout, dst.getHandle(), dstLayout,
        static_cast<uint32_t>(regions.size()), regions.data(), filter);
    return *this;
}

CommandBuffer& CommandBuffer::blitImage(
    const VkImage src, const VkImageLayout srcLayout, const VkImage dst, const VkImageLayout dstLayout,
    const std::span<VkImageBlit>& regions, const VkFilter filter)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBlitImage(
        commandBuffer_, src, srcLayout, dst, dstLayout, static_cast<uint32_t>(regions.size()), regions.data(),
        filter);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::memoryBarrier(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags, const VkMemoryBarrier& barrier)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 1, &barrier, 0, nullptr, 0, nullptr);
    return *this;
}

CommandBuffer& CommandBuffer::memoryBarrier(const VkDependencyFlags flags, const VkMemoryBarrier2& barrier)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = 1;
    info.pMemoryBarriers = &barrier;
    info.bufferMemoryBarrierCount = 0;
    info.pBufferMemoryBarriers = nullptr;
    info.imageMemoryBarrierCount = 0;
    info.pImageMemoryBarriers = nullptr;

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

CommandBuffer& CommandBuffer::memoryBarriers(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const std::span<VkMemoryBarrier>& barriers)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, static_cast<uint32_t>(barriers.size()),
        reinterpret_cast<const VkMemoryBarrier*>(barriers.data()), 0, nullptr, 0, nullptr);
    return *this;
}

CommandBuffer& CommandBuffer::memoryBarriers(
    const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& barriers)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = static_cast<uint32_t>(barriers.size());
    info.pMemoryBarriers = barriers.data();
    info.bufferMemoryBarrierCount = 0;
    info.pBufferMemoryBarriers = nullptr;
    info.imageMemoryBarrierCount = 0;
    info.pImageMemoryBarriers = nullptr;

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

CommandBuffer& CommandBuffer::bufferMemoryBarrier(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const VkBufferMemoryBarrier& barrier)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    return *this;
}
CommandBuffer& CommandBuffer::bufferMemoryBarrier(
    const VkDependencyFlags flags, const VkBufferMemoryBarrier2& barrier)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = 0;
    info.pMemoryBarriers = nullptr;
    info.bufferMemoryBarrierCount = 1;
    info.pBufferMemoryBarriers = &barrier;
    info.imageMemoryBarrierCount = 0;
    info.pImageMemoryBarriers = nullptr;

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

CommandBuffer& CommandBuffer::bufferMemoryBarriers(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const std::span<VkBufferMemoryBarrier>& barriers)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, static_cast<uint32_t>(barriers.size()),
        barriers.data(), 0, nullptr);
    return *this;
}

CommandBuffer& CommandBuffer::bufferMemoryBarriers(
    const VkDependencyFlags flags, const std::span<VkBufferMemoryBarrier2>& barriers)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = 0;
    info.pMemoryBarriers = nullptr;
    info.bufferMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
    info.pBufferMemoryBarriers = barriers.data();
    info.imageMemoryBarrierCount = 0;
    info.pImageMemoryBarriers = nullptr;

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

CommandBuffer& CommandBuffer::imageMemoryBarrier(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const VkImageMemoryBarrier& barrier)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    return *this;
}

CommandBuffer& CommandBuffer::imageMemoryBarrier(
    const VkDependencyFlags flags, const VkImageMemoryBarrier2& barrier)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = 0;
    info.pMemoryBarriers = nullptr;
    info.bufferMemoryBarrierCount = 0;
    info.pBufferMemoryBarriers = nullptr;
    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &barrier;

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

CommandBuffer& CommandBuffer::imageMemoryBarriers(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const std::span<VkImageMemoryBarrier>& barriers)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()),
        barriers.data());
    return *this;
}

CommandBuffer& CommandBuffer::imageMemoryBarriers(
    const VkDependencyFlags flags, const std::span<VkImageMemoryBarrier2>& barriers)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = 0;
    info.pMemoryBarriers = nullptr;
    info.bufferMemoryBarrierCount = 0;
    info.pBufferMemoryBarriers = nullptr;
    info.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
    info.pImageMemoryBarriers = barriers.data();

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

CommandBuffer& CommandBuffer::pipelineBarrier(
    const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const std::span<VkMemoryBarrier>& memoryBarriers,
    const std::span<VkBufferMemoryBarrier>& bufferMemoryBarriers,
    const std::span<VkImageMemoryBarrier>& imageMemoryBarriers)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdPipelineBarrier(
        commandBuffer_, srcFlags, dstFlags, 0, static_cast<uint32_t>(memoryBarriers.size()),
        reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.data()),
        static_cast<uint32_t>(bufferMemoryBarriers.size()),
        reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers.data()),
        static_cast<uint32_t>(imageMemoryBarriers.size()),
        reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers.data()));

    return *this;
}

CommandBuffer& CommandBuffer::pipelineBarrier(
    const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
    const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
    const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
    info.pMemoryBarriers = memoryBarriers.data();
    info.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferMemoryBarriers.size());
    info.pBufferMemoryBarriers = bufferMemoryBarriers.data();
    info.imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size());
    info.pImageMemoryBarriers = imageMemoryBarriers.data();

    device_->vk().vkCmdPipelineBarrier2(commandBuffer_, &info);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::setEvent(const Event& event, const VkPipelineStageFlags flags)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdSetEvent(commandBuffer_, event.getHandle(), flags);
    return *this;
}

CommandBuffer& CommandBuffer::setEvent(
    const Event& event, const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
    const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
    const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
    info.pMemoryBarriers = memoryBarriers.data();
    info.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferMemoryBarriers.size());
    info.pBufferMemoryBarriers = bufferMemoryBarriers.data();
    info.imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size());
    info.pImageMemoryBarriers = imageMemoryBarriers.data();

    device_->vk().vkCmdSetEvent2(commandBuffer_, event.getHandle(), &info);
    return *this;
}

CommandBuffer& CommandBuffer::waitEvent(
    const Event& event, const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
    const std::span<VkMemoryBarrier>& memoryBarriers,
    const std::span<VkBufferMemoryBarrier>& bufferMemoryBarriers,
    const std::span<VkImageMemoryBarrier>& imageMemoryBarriers)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdWaitEvents(
        commandBuffer_, 1, &event.getHandle(), srcFlags, dstFlags,
        static_cast<uint32_t>(memoryBarriers.size()),
        reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.data()),
        static_cast<uint32_t>(bufferMemoryBarriers.size()),
        reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers.data()),
        static_cast<uint32_t>(imageMemoryBarriers.size()),
        reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers.data()));
    return *this;
}

CommandBuffer& CommandBuffer::waitEvent(
    const Event& event, const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
    const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
    const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers)
{
    VKW_ASSERT(recording_);

    VkDependencyInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    info.pNext = nullptr;
    info.dependencyFlags = flags;
    info.memoryBarrierCount = static_cast<uint32_t>(memoryBarriers.size());
    info.pMemoryBarriers = memoryBarriers.data();
    info.bufferMemoryBarrierCount = static_cast<uint32_t>(bufferMemoryBarriers.size());
    info.pBufferMemoryBarriers = bufferMemoryBarriers.data();
    info.imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size());
    info.pImageMemoryBarriers = imageMemoryBarriers.data();

    device_->vk().vkCmdWaitEvents2(commandBuffer_, 1, &event.getHandle(), &info);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::bindComputePipeline(const ComputePipeline& pipeline)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
    return *this;
}

CommandBuffer& CommandBuffer::bindComputeDescriptorSet(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet)
{
    return bindComputeDescriptorSet(pipelineLayout, firstSet, descriptorSet.getHandle());
}

CommandBuffer& CommandBuffer::bindComputeDescriptorSet(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet, const VkDescriptorSet descriptorSet)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindDescriptorSets(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), firstSet, 1,
        &descriptorSet, 0, nullptr);

    return *this;
}

CommandBuffer& CommandBuffer::bindComputeDescriptorSets(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet,
    const std::vector<std::reference_wrapper<DescriptorSet>>& descriptorSets)
{
    auto descriptorSetList = utils::ScopedAllocator::allocateArray<VkDescriptorSet>(descriptorSets.size());

    size_t descriptorIndex = 0;
    for(const auto& descriptorSet : descriptorSets)
    {
        descriptorSetList[descriptorIndex++] = descriptorSet.get().getHandle();
    }
    return bindComputeDescriptorSets(pipelineLayout, firstSet, descriptorSetList);
}

CommandBuffer& CommandBuffer::bindComputeDescriptorSets(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet,
    const std::span<VkDescriptorSet>& descriptorSets)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindDescriptorSets(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), firstSet,
        static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
    return *this;
}

CommandBuffer& CommandBuffer::pushConstants(
    const PipelineLayout& pipelineLayout, const void* values, const uint32_t size, const ShaderStage stage)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdPushConstants(
        commandBuffer_, pipelineLayout.getHandle(), PipelineLayout::getVkShaderStage(stage), 0, size, values);

    return *this;
}

CommandBuffer& CommandBuffer::dispatch(uint32_t x, uint32_t y, uint32_t z)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDispatch(commandBuffer_, x, y, z);
    return *this;
}

CommandBuffer& CommandBuffer::dispatchIndirect(const BaseBuffer& dispatchBuffer, const VkDeviceSize offset)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDispatchIndirect(commandBuffer_, dispatchBuffer.getHandle(), offset);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeSamplerDescriptor(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkSampler sampler)
{
    const VkDescriptorImageInfo imgInfo = {sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeCombinedImageSampler(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkSampler sampler,
    const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {sampler, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeSampledImage(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeStorageImage(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeUniformTexelBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeStorageTexelBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeStorageBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeUniformBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeStorageBufferDynamic(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeUniformBufferDynamic(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushComputeAccelerationStructure(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkAccelerationStructureKHR accelerationStructure)
{
    VkWriteDescriptorSetAccelerationStructureKHR asWriteDescriptorSet = {};
    asWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    asWriteDescriptorSet.pNext = nullptr;
    asWriteDescriptorSet.accelerationStructureCount = 1;
    asWriteDescriptorSet.pAccelerationStructures = &accelerationStructure;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = &asWriteDescriptorSet;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::beginRenderPass(
    const RenderPass& renderPass, const VkFramebuffer frameBuffer, const VkOffset2D& offset,
    const VkExtent2D& extent, const VkClearColorValue& clearColor, const VkSubpassContents contents)
{
    VKW_ASSERT(recording_);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = renderPass.getHandle();
    renderPassInfo.framebuffer = frameBuffer;
    renderPassInfo.renderArea.offset = offset;
    renderPassInfo.renderArea.extent = extent;

    VkClearValue clearValues[2];
    clearValues[0].color = clearColor;
    if(renderPass.useDepth()) { clearValues[1].depthStencil = {1.0f, 0}; }

    renderPassInfo.clearValueCount = renderPass.useDepth() ? 2 : 1;
    renderPassInfo.pClearValues = clearValues;

    device_->vk().vkCmdBeginRenderPass(commandBuffer_, &renderPassInfo, contents);
    return *this;
}

CommandBuffer& CommandBuffer::nextSubpass(const VkSubpassContents contents)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdNextSubpass(commandBuffer_, contents);
    return *this;
}

CommandBuffer& CommandBuffer::endRenderPass()
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdEndRenderPass(commandBuffer_);
    return *this;
}

CommandBuffer& CommandBuffer::beginRendering(
    const RenderingAttachment& colorAttachment, const VkRect2D renderArea, const uint32_t viewMask,
    const uint32_t layerCount, const VkRenderingFlags flags)
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

CommandBuffer& CommandBuffer::beginRendering(
    const std::span<RenderingAttachment>& colorAttachments, const VkRect2D renderArea,
    const uint32_t viewMask, const uint32_t layerCount, const VkRenderingFlags flags)
{
    VKW_ASSERT(recording_);

    auto attachmentInfos
        = utils::ScopedAllocator::allocateArray<VkRenderingAttachmentInfo>(colorAttachments.size());

    size_t attachmentIndex = 0;
    for(const auto& colorAttachment : colorAttachments)
    {
        VkRenderingAttachmentInfo& attachmentInfo = attachmentInfos[attachmentIndex];
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
        attachmentIndex++;
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

CommandBuffer& CommandBuffer::beginRendering(
    RenderingAttachment& colorAttachment, RenderingAttachment& depthStencilAttachment,
    const VkRect2D renderArea, const uint32_t viewMask, const uint32_t layerCount,
    const VkRenderingFlags flags)
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

CommandBuffer& CommandBuffer::beginRendering(
    const std::span<RenderingAttachment>& colorAttachments, RenderingAttachment& depthStencilAttachment,
    const VkRect2D renderArea, const uint32_t viewMask, const uint32_t layerCount,
    const VkRenderingFlags flags)
{
    VKW_ASSERT(recording_);

    auto attachmentInfos
        = utils::ScopedAllocator::allocateArray<VkRenderingAttachmentInfo>(colorAttachments.size());

    size_t attachmentIndex = 0;
    for(const auto& colorAttachment : colorAttachments)
    {
        VkRenderingAttachmentInfo& attachmentInfo = attachmentInfos[attachmentIndex];
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
        attachmentIndex++;
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

CommandBuffer& CommandBuffer::endRendering()
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdEndRendering(commandBuffer_);
    return *this;
}

CommandBuffer& CommandBuffer::bindGraphicsPipeline(GraphicsPipeline& pipeline)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
    return *this;
}

CommandBuffer& CommandBuffer::bindGraphicsDescriptorSet(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet)
{
    return bindGraphicsDescriptorSet(pipelineLayout, firstSet, descriptorSet.getHandle());
}

CommandBuffer& CommandBuffer::bindGraphicsDescriptorSet(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet, const VkDescriptorSet descriptorSet)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindDescriptorSets(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), firstSet, 1,
        &descriptorSet, 0, nullptr);

    return *this;
}

CommandBuffer& CommandBuffer::bindGraphicsDescriptorSets(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet,
    const std::vector<std::reference_wrapper<DescriptorSet>>& descriptorSets)
{
    auto descriptorSetList = utils::ScopedAllocator::allocateArray<VkDescriptorSet>(descriptorSets.size());

    size_t descriptorIndex = 0;
    for(const auto& descriptorSet : descriptorSets)
    {
        descriptorSetList[descriptorIndex++] = descriptorSet.get().getHandle();
    }
    return bindGraphicsDescriptorSets(pipelineLayout, firstSet, descriptorSetList);
}

CommandBuffer& CommandBuffer::bindGraphicsDescriptorSets(
    const PipelineLayout& pipelineLayout, const uint32_t firstSet,
    const std::span<VkDescriptorSet>& descriptorSets)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindDescriptorSets(
        commandBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout.getHandle(), firstSet,
        static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsSamplerDescriptor(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkSampler sampler)
{
    const VkDescriptorImageInfo imgInfo = {sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsCombinedImageSampler(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkSampler sampler,
    const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {sampler, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsSampledImage(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsStorageImage(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkImageView imageView, const VkImageLayout layout)
{
    const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet.pImageInfo = &imgInfo;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsUniformTexelBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsStorageTexelBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkBufferView& bufferView)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = &bufferView;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsStorageBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsUniformBuffer(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsStorageBufferDynamic(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsUniformBufferDynamic(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding, const VkBuffer buffer,
    const VkDeviceSize offset, const VkDeviceSize range)
{
    const VkDescriptorBufferInfo bufferInfo = {buffer, offset, range};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

CommandBuffer& CommandBuffer::pushGraphicsAccelerationStructure(
    const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
    const VkAccelerationStructureKHR accelerationStructure)
{
    VkWriteDescriptorSetAccelerationStructureKHR asWriteDescriptorSet = {};
    asWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    asWriteDescriptorSet.pNext = nullptr;
    asWriteDescriptorSet.accelerationStructureCount = 1;
    asWriteDescriptorSet.pAccelerationStructures = &accelerationStructure;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = &asWriteDescriptorSet;
    writeDescriptorSet.dstSet = VK_NULL_HANDLE;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    device_->vk().vkCmdPushDescriptorSet(
        commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout.getHandle(), set, 1,
        &writeDescriptorSet);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::setViewport(
    const float offX, const float offY, const float width, const float height, const float minDepth,
    const float maxDepth)
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

CommandBuffer& CommandBuffer::setViewport(const VkViewport& viewport)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);
    return *this;
}

CommandBuffer& CommandBuffer::setViewport(const std::span<VkViewport>& viewports, const uint32_t offset)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetViewport(
        commandBuffer_, offset, static_cast<uint32_t>(viewports.size()), viewports.data());
    return *this;
}

CommandBuffer& CommandBuffer::setScissor(const VkOffset2D& offset, const VkExtent2D& extent)
{
    VKW_ASSERT(recording_);

    VkRect2D scissor{};
    scissor.offset = offset;
    scissor.extent = extent;

    device_->vk().vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
    return *this;
}

CommandBuffer& CommandBuffer::setScissor(const VkRect2D& scissor)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);
    return *this;
}

CommandBuffer& CommandBuffer::setScissor(const std::span<VkRect2D>& scissors, const uint32_t offset)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetScissor(
        commandBuffer_, offset, static_cast<uint32_t>(scissors.size()), scissors.data());
    return *this;
}

CommandBuffer& CommandBuffer::setLineWidth(const float lineWidth)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetLineWidth(commandBuffer_, lineWidth);
    return *this;
}

CommandBuffer& CommandBuffer::setDepthBias(
    const float depthBiasConstantFactor, const float depthBiasClamp, const float depthBiasSlopeFactor)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetDepthBias(
        commandBuffer_, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    return *this;
}

CommandBuffer& CommandBuffer::setBlendConstants(const float r, const float g, const float b, const float a)
{
    VKW_ASSERT(recording_);
    const float value[] = {r, g, b, a};
    device_->vk().vkCmdSetBlendConstants(commandBuffer_, value);
    return *this;
}

CommandBuffer& CommandBuffer::setStencilCompareMask(
    const VkStencilFaceFlags faceMask, const uint32_t compareMask)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetStencilCompareMask(commandBuffer_, faceMask, compareMask);
    return *this;
}

CommandBuffer& CommandBuffer::setStencilWriteMask(const VkStencilFaceFlags faceMask, const uint32_t writeMask)
{
    VKW_ASSERT(recording_);

    device_->vk().vkCmdSetStencilWriteMask(commandBuffer_, faceMask, writeMask);

    return *this;
}

CommandBuffer& CommandBuffer::setStencilReference(const VkStencilFaceFlags faceMask, const uint32_t reference)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetStencilReference(commandBuffer_, faceMask, reference);
    return *this;
}

CommandBuffer& CommandBuffer::setCullMode(const VkCullModeFlags cullMode)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetCullMode(commandBuffer_, cullMode);
    return *this;
}

CommandBuffer& CommandBuffer::setFrontFace(const VkFrontFace frontFace)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetFrontFace(commandBuffer_, frontFace);
    return *this;
}

CommandBuffer& CommandBuffer::setPrimitiveTopology(const VkPrimitiveTopology topology)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetPrimitiveTopology(commandBuffer_, topology);
    return *this;
}

CommandBuffer& CommandBuffer::setViewportWithCount(const std::span<VkViewport>& viewports)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetViewportWithCount(
        commandBuffer_, static_cast<uint32_t>(viewports.size()), viewports.data());
    return *this;
}

CommandBuffer& CommandBuffer::setScissorWithCount(const std::span<VkRect2D>& scissors)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetScissorWithCount(
        commandBuffer_, static_cast<uint32_t>(scissors.size()), scissors.data());
    return *this;
}

CommandBuffer& CommandBuffer::CommandBuffer::setDepthTestEnable(const VkBool32 depthTestEnable)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetDepthTestEnable(commandBuffer_, depthTestEnable);
    return *this;
}

CommandBuffer& CommandBuffer::setDepthWriteEnable(const VkBool32 depthWriteEnable)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetDepthWriteEnable(commandBuffer_, depthWriteEnable);
    return *this;
}

CommandBuffer& CommandBuffer::setDepthCompareOp(const VkCompareOp compareOp)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetDepthCompareOp(commandBuffer_, compareOp);
    return *this;
}

CommandBuffer& CommandBuffer::setDepthBoundsTestEnable(const VkBool32 depthBoundsTestEnable)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetDepthBoundsTestEnable(commandBuffer_, depthBoundsTestEnable);
    return *this;
}

CommandBuffer& CommandBuffer::setStencilTestEnable(const VkBool32 stencilTestEnable)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetStencilTestEnable(commandBuffer_, stencilTestEnable);
    return *this;
}

CommandBuffer& CommandBuffer::setStencilOp(
    const VkStencilFaceFlags faceMask, const VkStencilOp failOp, const VkStencilOp passOp,
    const VkStencilOp depthFailOp, const VkCompareOp compareOp)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetStencilOp(commandBuffer_, faceMask, failOp, passOp, depthFailOp, compareOp);
    return *this;
}

CommandBuffer& CommandBuffer::setDepthBiasEnable(const VkBool32 depthBiasEnable)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdSetDepthBiasEnable(commandBuffer_, depthBiasEnable);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::bindVertexBuffer(
    const uint32_t binding, const BaseBuffer& buffer, const VkDeviceSize offset)
{
    VKW_ASSERT(recording_);
    const VkBuffer bufferHandle = buffer.getHandle();
    const VkDeviceSize offsetBytes = offset * buffer.stride();
    device_->vk().vkCmdBindVertexBuffers(commandBuffer_, binding, 1, &bufferHandle, &offsetBytes);
    return *this;
}

CommandBuffer& CommandBuffer::bindVertexBuffer(
    const uint32_t binding, const VkBuffer& buffer, const VkDeviceSize byteOffset)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindVertexBuffers(commandBuffer_, binding, 1, &buffer, &byteOffset);
    return *this;
}

CommandBuffer& CommandBuffer::bindVertexBuffers(
    const uint32_t firstBinding, const std::span<VkBuffer>& buffers,
    const std::span<VkDeviceSize>& byteOffsets)
{
    VKW_ASSERT(recording_);
    VKW_ASSERT(buffers.size() == byteOffsets.size());
    device_->vk().vkCmdBindVertexBuffers(
        commandBuffer_, firstBinding, static_cast<uint32_t>(buffers.size()), buffers.data(),
        byteOffsets.data());
    return *this;
}

CommandBuffer& CommandBuffer::bindIndexBuffer(
    const BaseBuffer& buffer, const VkIndexType indexType, const VkDeviceSize byteOffset)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindIndexBuffer(commandBuffer_, buffer.getHandle(), byteOffset, indexType);
    return *this;
}

CommandBuffer& CommandBuffer::bindIndexBuffer(
    const VkBuffer& buffer, const VkIndexType indexType, const VkDeviceSize byteOffset)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdBindIndexBuffer(commandBuffer_, buffer, byteOffset, indexType);
    return *this;
}

CommandBuffer& CommandBuffer::draw(
    const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
    const uint32_t firstInstance)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDraw(commandBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndirect(
    const BaseBuffer& indirectBuffer, const uint32_t drawCount, const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndirect(commandBuffer_, indirectBuffer.getHandle(), 0, drawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndirect(
    const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const uint32_t drawCount,
    const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndirect(
        commandBuffer_, indirectBuffer.getHandle(), offsetBytes, drawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndirectCount(
    const BaseBuffer& indirectBuffer, const BaseBuffer& countBuffer, const uint32_t maxDrawCount,
    const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndirectCount(
        commandBuffer_, indirectBuffer.getHandle(), 0, countBuffer.getHandle(), 0, maxDrawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndirectCount(
    const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const BaseBuffer& countBuffer,
    const size_t countOffsetBytes, const uint32_t maxDrawCount, const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndirectCount(
        commandBuffer_, indirectBuffer.getHandle(), offsetBytes, countBuffer.getHandle(), countOffsetBytes,
        maxDrawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndexed(
    const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
    const uint32_t vertexOffset, const uint32_t firstInstance)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndexed(
        commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndexedIndirect(
    const BaseBuffer& indirectBuffer, const uint32_t drawCount, const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndexedIndirect(commandBuffer_, indirectBuffer.getHandle(), 0, drawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndexedIndirect(
    const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const uint32_t drawCount,
    const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndexedIndirect(
        commandBuffer_, indirectBuffer.getHandle(), offsetBytes, drawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndexedIndirectCount(
    const BaseBuffer& indirectBuffer, const BaseBuffer& countBuffer, const uint32_t maxDrawCount,
    const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndexedIndirectCount(
        commandBuffer_, indirectBuffer.getHandle(), 0, countBuffer.getHandle(), 0, maxDrawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawIndexedIndirectCount(
    const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const BaseBuffer& countBuffer,
    const size_t countOffsetBytes, const uint32_t maxDrawCount, const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawIndexedIndirectCount(
        commandBuffer_, indirectBuffer.getHandle(), offsetBytes, countBuffer.getHandle(), countOffsetBytes,
        maxDrawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawMeshTasks(
    const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawMeshTasksEXT(commandBuffer_, groupCountX, groupCountY, groupCountZ);
    return *this;
}

CommandBuffer& CommandBuffer::drawMeshTasksIndirect(
    const BaseBuffer& buffer, const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawMeshTasksIndirectEXT(
        commandBuffer_, buffer.getHandle(), offset, drawCount, stride);
    return *this;
}

CommandBuffer& CommandBuffer::drawMeshTasksIndirectCount(
    const BaseBuffer& buffer, const VkDeviceSize offset, const BaseBuffer& countBuffer,
    const VkDeviceSize countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride)
{
    VKW_ASSERT(recording_);
    device_->vk().vkCmdDrawMeshTasksIndirectCountEXT(
        commandBuffer_, buffer.getHandle(), offset, countBuffer.getHandle(), countBufferOffset, maxDrawCount,
        stride);
    return *this;
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::buildAccelerationStructure(
    const BottomLevelAccelerationStructure& blas, const BaseBuffer& scratchBuffer,
    const VkBuildAccelerationStructureFlagsKHR buildFlags)
{
    VKW_ASSERT(recording_);
    VKW_ASSERT(blas.buildOnHost_ == false);
    VKW_ASSERT(blas.geometryData_.size() == blas.buildRanges_.size());

    auto ppBuildRanges
        = utils::ScopedAllocator::allocateArray<const VkAccelerationStructureBuildRangeInfoKHR*>(
            blas.buildRanges_.size());

    size_t rangeIndex = 0;
    for(const auto& rangeList : blas.buildRanges_)
    {
        ppBuildRanges[rangeIndex++] = rangeList.data();
    }

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
    device_->vk().vkCmdBuildAccelerationStructuresKHR(commandBuffer_, 1, &buildInfo, ppBuildRanges.data());

    return *this;
}

CommandBuffer& CommandBuffer::buildAccelerationStructure(
    const TopLevelAccelerationStructure& tlas, const BaseBuffer& scratchBuffer,
    const VkBuildAccelerationStructureFlagsKHR buildFlags)
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
    device_->vk().vkCmdBuildAccelerationStructuresKHR(commandBuffer_, 1, &buildInfo, &pBuildRanges);

    return *this;
}

CommandBuffer& CommandBuffer::updateAccelerationStructure(
    TopLevelAccelerationStructure& tlas, const BaseBuffer& scratchBuffer,
    const VkBuildAccelerationStructureFlagsKHR buildFlags)
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
    buildInfo.srcAccelerationStructure = tlas.getHandle();
    buildInfo.dstAccelerationStructure = tlas.getHandle();
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries = &tlas.geometry_;
    buildInfo.ppGeometries = nullptr;
    buildInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress();
    device_->vk().vkCmdBuildAccelerationStructuresKHR(commandBuffer_, 1, &buildInfo, &pBuildRanges);

    return *this;
}

CommandBuffer& CommandBuffer::updateAccelerationStructure(
    TopLevelAccelerationStructure& tlas, const std::span<VkTransformMatrixKHR>& transforms,
    const BaseBuffer& scratchBuffer, const VkBuildAccelerationStructureFlagsKHR buildFlags)
{
    VKW_ASSERT(recording_);
    VKW_ASSERT(transforms.size() >= tlas.instancesList_.size());

    for(size_t i = 0; i < tlas.instancesList_.size(); ++i)
    {
        tlas.instancesList_[i].transform = transforms[i];
    }
    return this->updateAccelerationStructure(tlas, scratchBuffer, buildFlags);
}

// -----------------------------------------------------------------------------------------------------------

CommandBuffer& CommandBuffer::insertDebugMarker(const char* name, const float color[4])
{
    VKW_ASSERT(recording_);

    VkDebugUtilsLabelEXT markerInfo = {};
    markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    markerInfo.pNext = nullptr;
    markerInfo.pLabelName = name;
    markerInfo.color[0] = color[0];
    markerInfo.color[1] = color[1];
    markerInfo.color[2] = color[2];
    markerInfo.color[3] = color[3];

    static auto CmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT) vkGetInstanceProcAddr(
        device_->instance().getHandle(), "vkCmdInsertDebugUtilsLabelEXT");
    if(CmdInsertDebugUtilsLabelEXT != nullptr) { CmdInsertDebugUtilsLabelEXT(commandBuffer_, &markerInfo); }

    return *this;
}

CommandBuffer& CommandBuffer::beginDebugRegion(const char* name, const float color[4])
{
    VKW_ASSERT(recording_);

    VkDebugUtilsLabelEXT markerInfo = {};
    markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    markerInfo.pNext = nullptr;
    markerInfo.pLabelName = name;
    markerInfo.color[0] = color[0];
    markerInfo.color[1] = color[1];
    markerInfo.color[2] = color[2];
    markerInfo.color[3] = color[3];

    static auto CmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT) vkGetInstanceProcAddr(
        device_->instance().getHandle(), "vkCmdBeginDebugUtilsLabelEXT");
    if(CmdBeginDebugUtilsLabelEXT != nullptr) { CmdBeginDebugUtilsLabelEXT(commandBuffer_, &markerInfo); }

    return *this;
}

CommandBuffer& CommandBuffer::endDebugRegion()
{
    VKW_ASSERT(recording_);

    static auto CmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT) vkGetInstanceProcAddr(
        device_->instance().getHandle(), "vkCmdEndDebugUtilsLabelEXT");
    if(CmdEndDebugUtilsLabelEXT != nullptr) { CmdEndDebugUtilsLabelEXT(commandBuffer_); }

    return *this;
}

// -----------------------------------------------------------------------------------------------------------

VkMemoryBarrier createMemoryBarrier(const VkAccessFlags srcMask, const VkAccessFlags dstMask)
{
    VkMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;

    return ret;
}

VkMemoryBarrier2 createMemoryBarrier(
    const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask)
{
    VkMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    return ret;
}

VkBufferMemoryBarrier createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkDeviceSize offsetBytes, const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.buffer = buffer.getHandle();
    ret.offset = offsetBytes;
    ret.size = sizeBytes;

    return ret;
}

VkBufferMemoryBarrier createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex, const VkDeviceSize offsetBytes,
    const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.buffer = buffer.getHandle();
    ret.offset = offsetBytes;
    ret.size = sizeBytes;

    return ret;
}

VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkDeviceSize offsetBytes,
    const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.buffer = buffer.getHandle();
    ret.offset = offsetBytes;
    ret.size = sizeBytes;
    return ret;
}

VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkDeviceSize offsetBytes, const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.buffer = buffer.getHandle();
    ret.offset = offsetBytes;
    ret.size = sizeBytes;
    return ret;
}

VkBufferMemoryBarrier createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkDeviceSize offsetBytes, const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.buffer = buffer;
    ret.offset = offsetBytes;
    ret.size = sizeBytes;

    return ret;
}

VkBufferMemoryBarrier createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex, const VkDeviceSize offsetBytes,
    const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.buffer = buffer;
    ret.offset = offsetBytes;
    ret.size = sizeBytes;

    return ret;
}

VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkDeviceSize offsetBytes,
    const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.buffer = buffer;
    ret.offset = offsetBytes;
    ret.size = sizeBytes;
    return ret;
}

VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkDeviceSize offsetBytes, const VkDeviceSize sizeBytes)
{
    VkBufferMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.buffer = buffer;
    ret.offset = offsetBytes;
    ret.size = sizeBytes;
    return ret;
}

// -----------------------------------------------------------------------------------------------------------

VkImageMemoryBarrier createImageMemoryBarrier(
    const BaseImage& image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags,
    const uint32_t baseMipLevel, const uint32_t levelCount, const uint32_t baseArrayLayer,
    const uint32_t layerCount)
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

VkImageMemoryBarrier createImageMemoryBarrier(
    const BaseImage& image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkImageAspectFlags aspectFlags, const uint32_t baseMipLevel,
    const uint32_t levelCount, const uint32_t baseArrayLayer, const uint32_t layerCount)
{
    VkImageMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.image = image.getHandle();
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

VkImageMemoryBarrier2 createImageMemoryBarrier(
    const BaseImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags, const uint32_t baseMipLevel,
    const uint32_t levelCount, const uint32_t baseArrayLayer, const uint32_t layerCount)
{
    VkImageMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.image = image.getHandle();
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

VkImageMemoryBarrier2 createImageMemoryBarrier(
    const BaseImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex,
    const VkImageAspectFlags aspectFlags, const uint32_t baseMipLevel, const uint32_t levelCount,
    const uint32_t baseArrayLayer, const uint32_t layerCount)
{
    VkImageMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.image = image.getHandle();
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

VkImageMemoryBarrier createImageMemoryBarrier(
    const VkImage image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags,
    const uint32_t baseMipLevel, const uint32_t levelCount, const uint32_t baseArrayLayer,
    const uint32_t layerCount)
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

VkImageMemoryBarrier createImageMemoryBarrier(
    const VkImage image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkImageAspectFlags aspectFlags, const uint32_t baseMipLevel,
    const uint32_t levelCount, const uint32_t baseArrayLayer, const uint32_t layerCount)
{
    VkImageMemoryBarrier ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcAccessMask = srcMask;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.image = image;
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

VkImageMemoryBarrier2 createImageMemoryBarrier(
    const VkImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags, const uint32_t baseMipLevel,
    const uint32_t levelCount, const uint32_t baseArrayLayer, const uint32_t layerCount)
{
    VkImageMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ret.image = image;
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}

VkImageMemoryBarrier2 createImageMemoryBarrier(
    const VkImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex,
    const VkImageAspectFlags aspectFlags, const uint32_t baseMipLevel, const uint32_t levelCount,
    const uint32_t baseArrayLayer, const uint32_t layerCount)
{
    VkImageMemoryBarrier2 ret{};
    ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ret.pNext = nullptr;
    ret.srcStageMask = srcStages;
    ret.srcAccessMask = srcMask;
    ret.dstStageMask = dstStages;
    ret.dstAccessMask = dstMask;
    ret.oldLayout = oldLayout;
    ret.newLayout = newLayout;
    ret.srcQueueFamilyIndex = srcQueueFamilyIndex;
    ret.dstQueueFamilyIndex = dstQueueFamilyIndex;
    ret.image = image;
    ret.subresourceRange = {aspectFlags, baseMipLevel, levelCount, baseArrayLayer, layerCount};

    return ret;
}
} // namespace vkw