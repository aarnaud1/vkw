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

#include "vkw/detail/BottomLevelAS.hpp"
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
#include "vkw/detail/TopLevelAS.hpp"

#include <functional>
#include <span>

namespace vkw
{
class CommandBuffer
{
  public:
    constexpr CommandBuffer() {}

    CommandBuffer(
        const Device& device, VkCommandPool commandPool,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer(CommandBuffer&& cp) { *this = std::move(cp); }

    CommandBuffer& operator=(const CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&& cp);
    ~CommandBuffer() { this->clear(); }

    bool init(
        const Device& device, VkCommandPool commandPool,
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void clear();

    bool initialized() const { return initialized_; }

    bool begin(VkCommandBufferUsageFlags usage = 0);
    bool end();
    bool reset();

    // -------------------------------------------------------------------------------------------------------
    // ---------------------------- Transfer operations ------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& copyBuffer(
        const BaseBuffer& src, const BaseBuffer& dst, const std::span<VkBufferCopy>& regions);
    CommandBuffer& copyBuffer(const BaseBuffer& src, const BaseBuffer& dst);

    CommandBuffer& fillBuffer(
        const BaseBuffer& buffer, const uint32_t val, const size_t offset, const size_t size);

    CommandBuffer& copyBufferToImage(
        const BaseBuffer& buffer, const BaseImage& image, const VkImageLayout dstLayout,
        const VkBufferImageCopy& region);
    CommandBuffer& copyBufferToImage(
        const BaseBuffer& buffer, const BaseImage& image, VkImageLayout dstLayout,
        const std::span<VkBufferImageCopy>& regions);

    CommandBuffer& copyImageToBuffer(
        const BaseImage& image, VkImageLayout srcLayout, const BaseBuffer& buffer,
        const VkBufferImageCopy& region);
    CommandBuffer& copyImageToBuffer(
        const BaseImage& image, VkImageLayout srcLayout, const BaseBuffer& buffer,
        const std::span<VkBufferImageCopy>& regions);

    CommandBuffer& blitImage(
        const BaseImage& src, const VkImageLayout srcLayout, const BaseImage& dst,
        const VkImageLayout dstLayout, const VkImageBlit region, const VkFilter filter = VK_FILTER_LINEAR);
    CommandBuffer& blitImage(
        const VkImage src, const VkImageLayout srcLayout, const VkImage dst, const VkImageLayout dstLayout,
        const VkImageBlit region, const VkFilter filter = VK_FILTER_LINEAR);
    CommandBuffer& blitImage(
        const BaseImage& src, const VkImageLayout srcLayout, const BaseImage& dst,
        const VkImageLayout dstLayout, const std::span<VkImageBlit>& regions,
        const VkFilter filter = VK_FILTER_LINEAR);
    CommandBuffer& blitImage(
        const VkImage src, const VkImageLayout srcLayout, const VkImage dst, const VkImageLayout dstLayout,
        const std::span<VkImageBlit>& regions, const VkFilter filter = VK_FILTER_LINEAR);

    // -------------------------------------------------------------------------------------------------------
    // ----------------------------------- Pipeline barriers -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& memoryBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const VkMemoryBarrier& barrier);
    CommandBuffer& memoryBarrier(const VkDependencyFlags flags, const VkMemoryBarrier2& barrier);
    CommandBuffer& memoryBarrier(const VkMemoryBarrier2& barrier) { return memoryBarrier({}, barrier); }

    CommandBuffer& memoryBarriers(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkMemoryBarrier>& barriers);
    CommandBuffer& memoryBarriers(const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& barriers);
    CommandBuffer& memoryBarriers(const std::span<VkMemoryBarrier2>& barriers)
    {
        return memoryBarriers({}, barriers);
    }

    CommandBuffer& bufferMemoryBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const VkBufferMemoryBarrier& barrier);
    CommandBuffer& bufferMemoryBarrier(const VkDependencyFlags flags, const VkBufferMemoryBarrier2& barrier);
    CommandBuffer& bufferMemoryBarrier(const VkBufferMemoryBarrier2& barrier)
    {
        return bufferMemoryBarrier({}, barrier);
    }

    CommandBuffer& bufferMemoryBarriers(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkBufferMemoryBarrier>& barriers);
    CommandBuffer& bufferMemoryBarriers(
        const VkDependencyFlags flags, const std::span<VkBufferMemoryBarrier2>& barriers);
    CommandBuffer& bufferMemoryBarrier(const std::span<VkBufferMemoryBarrier2>& barriers)
    {
        return bufferMemoryBarriers({}, barriers);
    }

    CommandBuffer& imageMemoryBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const VkImageMemoryBarrier& barrier);
    CommandBuffer& imageMemoryBarrier(const VkDependencyFlags flags, const VkImageMemoryBarrier2& barrier);
    CommandBuffer& imageMemoryBarrier(const VkImageMemoryBarrier2& barrier)
    {
        return imageMemoryBarrier({}, barrier);
    }

    CommandBuffer& imageMemoryBarriers(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkImageMemoryBarrier>& barriers);
    CommandBuffer& imageMemoryBarriers(
        const VkDependencyFlags flags, const std::span<VkImageMemoryBarrier2>& barriers);
    CommandBuffer& imageMemoryBarriers(const std::span<VkImageMemoryBarrier2>& barriers)
    {
        return imageMemoryBarriers({}, barriers);
    }

    CommandBuffer& pipelineBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkMemoryBarrier>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier>& imageMemoryBarriers);
    CommandBuffer& pipelineBarrier(
        const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers);
    CommandBuffer& pipelineBarrier(
        const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers)
    {
        return pipelineBarrier({}, memoryBarriers, bufferMemoryBarriers, imageMemoryBarriers);
    }

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------------ Events -----------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& setEvent(const Event& event, const VkPipelineStageFlags flags);
    CommandBuffer& setEvent(
        const Event& event, const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers);

    CommandBuffer& waitEvent(
        const Event& event, const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkMemoryBarrier>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier>& imageMemoryBarriers);
    CommandBuffer& waitEvent(
        const Event& event, const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers);

    // -------------------------------------------------------------------------------------------------------
    // ----------------------------- Compute pipelines -------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& bindComputePipeline(const ComputePipeline& pipeline);

    CommandBuffer& bindComputeDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet);
    CommandBuffer& bindComputeDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const VkDescriptorSet descriptorSet);

    CommandBuffer& bindComputeDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::initializer_list<std::reference_wrapper<DescriptorSet>>& descriptorSets);
    CommandBuffer& bindComputeDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::span<VkDescriptorSet>& descriptorSets);

    template <typename T>
    CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout, const T& values, const ShaderStage stage)
    {
        return pushConstants(pipelineLayout, &values, sizeof(T), stage);
    }
    CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout, const void* values, const uint32_t size,
        const ShaderStage stage);

    template <typename T, typename... Args>
    CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout, const ShaderStage stage, const T& values, Args&&... stages)
    {
        pushConstants(pipelineLayout, values, stage);
        return pushConstants(pipelineLayout, values, std::forward<Args>(stages)...);
    }

    CommandBuffer& dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1);
    CommandBuffer& dispatchIndirect(const BaseBuffer& dispatchBuffer, const VkDeviceSize offset = 0);

    CommandBuffer& pushComputeSamplerDescriptor(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler);

    CommandBuffer& pushComputeCombinedImageSampler(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler, const VkImageView imageView, const VkImageLayout layout);

    CommandBuffer& pushComputeSampledImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout);

    CommandBuffer& pushComputeStorageImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout);

    CommandBuffer& pushComputeUniformTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView);

    CommandBuffer& pushComputeStorageTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView);

    CommandBuffer& pushComputeStorageBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushComputeUniformBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushComputeStorageBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushComputeUniformBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushComputeAccelerationStructure(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkAccelerationStructureKHR accelerationStructure);

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------- Graphics pipeline -----------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& beginRenderPass(
        const RenderPass& renderPass, const VkFramebuffer frameBuffer, const VkOffset2D& offset,
        const VkExtent2D& extent, const VkClearColorValue& clearColor,
        const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

    CommandBuffer& nextSubpass(const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    CommandBuffer& endRenderPass();

    CommandBuffer& beginRendering(
        const RenderingAttachment& colorAttachment, const VkRect2D renderArea, const uint32_t viewMask = 0,
        const uint32_t layerCount = 1, const VkRenderingFlags flags = 0);
    CommandBuffer& beginRendering(
        const std::span<RenderingAttachment>& colorAttachments, const VkRect2D renderArea,
        const uint32_t viewMask = 0, const uint32_t layerCount = 1, const VkRenderingFlags flags = 0);
    CommandBuffer& beginRendering(
        RenderingAttachment& colorAttachment, RenderingAttachment& depthStencilAttachment,
        const VkRect2D renderArea, const uint32_t viewMask = 0, const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0);
    CommandBuffer& beginRendering(
        const std::span<RenderingAttachment>& colorAttachments, RenderingAttachment& depthStencilAttachment,
        const VkRect2D renderArea, const uint32_t viewMask = 0, const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0);

    CommandBuffer& endRendering();

    CommandBuffer& bindGraphicsPipeline(GraphicsPipeline& pipeline);

    CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet);
    CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const VkDescriptorSet descriptorSet);

    CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::initializer_list<std::reference_wrapper<DescriptorSet>>& descriptorSets);
    CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet)
    {
        return bindGraphicsDescriptorSet(pipelineLayout, firstSet, descriptorSet);
    }
    template <typename... Args>
    CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet,
        Args&&... args)
    {
        bindGraphicsDescriptorSets(pipelineLayout, firstSet, descriptorSet);
        return bindGraphicsDescriptorSets(pipelineLayout, firstSet + 1, std::forward<Args>(args)...);
    }
    CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::span<VkDescriptorSet>& descriptorSets);

    CommandBuffer& pushGraphicsSamplerDescriptor(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler);

    CommandBuffer& pushGraphicsCombinedImageSampler(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler, const VkImageView imageView, const VkImageLayout layout);

    CommandBuffer& pushGraphicsSampledImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout);

    CommandBuffer& pushGraphicsStorageImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout);

    CommandBuffer& pushGraphicsUniformTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView);

    CommandBuffer& pushGraphicsStorageTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView);

    CommandBuffer& pushGraphicsStorageBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushGraphicsUniformBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushGraphicsStorageBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushGraphicsUniformBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range);

    CommandBuffer& pushGraphicsAccelerationStructure(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkAccelerationStructureKHR accelerationStructure);

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------------- Dynamic states -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& setViewport(
        const float offX, const float offY, const float width, const float height,
        const float minDepth = 0.0f, const float maxDepth = 1.0f);
    CommandBuffer& setViewport(const VkViewport& viewport);
    CommandBuffer& setViewport(const std::span<VkViewport>& viewports, const uint32_t offset = 0);

    CommandBuffer& setScissor(const VkOffset2D& offset, const VkExtent2D& extent);
    CommandBuffer& setScissor(const VkRect2D& scissor);
    CommandBuffer& setScissor(const std::span<VkRect2D>& scissors, const uint32_t offset = 0);

    CommandBuffer& setLineWidth(const float lineWidth);

    CommandBuffer& setDepthBias(
        const float depthBiasConstantFactor, const float depthBiasClamp, const float depthBiasSlopeFactor);

    CommandBuffer& setBlendConstants(const float r, const float g, const float b, const float a);

    CommandBuffer& setStencilCompareMask(const VkStencilFaceFlags faceMask, const uint32_t compareMask);

    CommandBuffer& setStencilWriteMask(const VkStencilFaceFlags faceMask, const uint32_t writeMask);

    CommandBuffer& setStencilReference(const VkStencilFaceFlags faceMask, const uint32_t reference);

    CommandBuffer& setCullMode(const VkCullModeFlags cullMode);

    CommandBuffer& setFrontFace(const VkFrontFace frontFace);

    CommandBuffer& setPrimitiveTopology(const VkPrimitiveTopology topology);

    CommandBuffer& setViewportWithCount(const std::span<VkViewport>& viewports);

    CommandBuffer& setScissorWithCount(const std::span<VkRect2D>& scissors);

    CommandBuffer& setDepthTestEnable(const VkBool32 depthTestEnable);

    CommandBuffer& setDepthWriteEnable(const VkBool32 depthWriteEnable);

    CommandBuffer& setDepthCompareOp(const VkCompareOp compareOp);

    CommandBuffer& setDepthBoundsTestEnable(const VkBool32 depthBoundsTestEnable);

    CommandBuffer& setStencilTestEnable(const VkBool32 stencilTestEnable);

    CommandBuffer& setStencilOp(
        const VkStencilFaceFlags faceMask, const VkStencilOp failOp, const VkStencilOp passOp,
        const VkStencilOp depthFailOp, const VkCompareOp compareOp);

    CommandBuffer& setDepthBiasEnable(const VkBool32 depthBiasEnable);

    // -------------------------------------------------------------------------------------------------------
    // ----------------------------------------- Drawing -----------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& bindVertexBuffer(
        const uint32_t binding, const BaseBuffer& buffer, const VkDeviceSize offset);
    CommandBuffer& bindVertexBuffer(
        const uint32_t binding, const VkBuffer& buffer, const VkDeviceSize byteOffset);

    CommandBuffer& bindVertexBuffers(
        const uint32_t firstBinding, const std::tuple<const BaseBuffer&, const VkDeviceSize>& bufferData)
    {
        const auto& [buffer, size] = bufferData;
        return bindVertexBuffer(firstBinding, buffer, size);
    }
    template <typename... Args>
    CommandBuffer& bindVertexBuffers(
        const uint32_t firstBinding, const std::tuple<const BaseBuffer&, const VkDeviceSize>& bufferData,
        Args&&... args)
    {
        bindVertexBuffers(firstBinding, bufferData);
        return bindVertexBuffers(firstBinding + 1, std::forward<Args>(args)...);
    }
    CommandBuffer& bindVertexBuffers(
        const uint32_t firstBinding, const std::span<VkBuffer>& buffers,
        const std::span<VkDeviceSize>& byteOffsets);

    CommandBuffer& bindIndexBuffer(
        const BaseBuffer& buffer, const VkIndexType indexType, const VkDeviceSize byteOffset = 0);
    CommandBuffer& bindIndexBuffer(
        const VkBuffer& buffer, const VkIndexType indexType, const VkDeviceSize byteOffset = 0);

    CommandBuffer& draw(
        const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
        const uint32_t firstInstance);
    CommandBuffer& drawIndirect(
        const BaseBuffer& indirectBuffer, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand));
    CommandBuffer& drawIndirect(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand));
    CommandBuffer& drawIndirectCount(
        const BaseBuffer& indirectBuffer, const BaseBuffer& countBuffer, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand));
    CommandBuffer& drawIndirectCount(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const BaseBuffer& countBuffer,
        const size_t countOffsetBytes, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand));

    CommandBuffer& drawIndexed(
        const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
        const uint32_t vertexOffset, const uint32_t firstInstance);
    CommandBuffer& drawIndexedIndirect(
        const BaseBuffer& indirectBuffer, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand));
    CommandBuffer& drawIndexedIndirect(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand));
    CommandBuffer& drawIndexedIndirectCount(
        const BaseBuffer& indirectBuffer, const BaseBuffer& countBuffer, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand));
    CommandBuffer& drawIndexedIndirectCount(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const BaseBuffer& countBuffer,
        const size_t countOffsetBytes, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand));

    CommandBuffer& drawMeshTasks(
        const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ);
    CommandBuffer& drawMeshTasksIndirect(
        const BaseBuffer& buffer, const VkDeviceSize offset, const uint32_t drawCount, const uint32_t stride);
    CommandBuffer& drawMeshTasksIndirectCount(
        const BaseBuffer& buffer, const VkDeviceSize offset, const BaseBuffer& countBuffer,
        const VkDeviceSize countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride);

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------- Acceleration structures -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    CommandBuffer& buildAccelerationStructure(
        const BottomLevelAccelerationStructure& blas, const BaseBuffer& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {});
    CommandBuffer& buildAccelerationStructure(
        const TopLevelAccelerationStructure& tlas, const BaseBuffer& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {});

    CommandBuffer& updateAccelerationStructure(
        TopLevelAccelerationStructure& tlas, const BaseBuffer& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {});
    CommandBuffer& updateAccelerationStructure(
        TopLevelAccelerationStructure& tlas, const std::span<VkTransformMatrixKHR>& transforms,
        const BaseBuffer& scratchBuffer, const VkBuildAccelerationStructureFlagsKHR buildFlags = {});

    ///@todo Implement buildAccelerationStructures()
    ///@todo Implement buildAccelerationStructureIndirect
    ///@todo Implement buildAccelerationStructuresIndirect()
    ///@todo Implement copyAccelerationStructure()

    // -------------------------------------------------------------------------------------------------------

    ///@todo Implement setRayTracingPipelineStackSize()
    ///@todo Implement traceRaysIndirect()
    ///@todo Implement traceRays()

    // -------------------------------------------------------------------------------------------------------

    VkCommandBuffer getHandle() const { return commandBuffer_; }

  private:
    const Device* device_{nullptr};
    VkCommandPool cmdPool_{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};

    bool recording_{false};
    bool initialized_{false};
};

// -----------------------------------------------------------------------------------------------------------
// ---------------------------------- Memory barriers --------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

[[nodiscard]] VkMemoryBarrier createMemoryBarrier(const VkAccessFlags srcMask, const VkAccessFlags dstMask);
[[nodiscard]] VkMemoryBarrier2 createMemoryBarrier(
    const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask);

// -----------------------------------------------------------------------------------------------------------
// ---------------------------------- Buffer memory barriers -------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

[[nodiscard]] VkBufferMemoryBarrier createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkDeviceSize offsetBytes = 0, const VkDeviceSize sizeBytes = VK_WHOLE_SIZE);
[[nodiscard]] VkBufferMemoryBarrier createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex,
    const VkDeviceSize offsetBytes = 0, const VkDeviceSize sizeBytes = VK_WHOLE_SIZE);
[[nodiscard]] VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkDeviceSize offsetBytes = 0,
    const VkDeviceSize sizeBytes = VK_WHOLE_SIZE);
[[nodiscard]] VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const BaseBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkDeviceSize offsetBytes = 0,
    const VkDeviceSize size = VK_WHOLE_SIZE);

[[nodiscard]] VkBufferMemoryBarrier createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkDeviceSize offsetBytes = 0, const VkDeviceSize sizeBytes = VK_WHOLE_SIZE);
[[nodiscard]] VkBufferMemoryBarrier createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex,
    const VkDeviceSize offsetBytes = 0, const VkDeviceSize sizeBytes = VK_WHOLE_SIZE);
[[nodiscard]] VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkDeviceSize offsetBytes = 0,
    const VkDeviceSize sizeBytes = VK_WHOLE_SIZE);
[[nodiscard]] VkBufferMemoryBarrier2 createBufferMemoryBarrier(
    const VkBuffer& buffer, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkDeviceSize offsetBytes = 0,
    const VkDeviceSize size = VK_WHOLE_SIZE);

// -----------------------------------------------------------------------------------------------------------
// ---------------------------------- Image memory barriers --------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

[[nodiscard]] VkImageMemoryBarrier createImageMemoryBarrier(
    const BaseImage& image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout,
    const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, const uint32_t baseMipLevel = 0,
    const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0, const uint32_t layerCount = 1);
[[nodiscard]] VkImageMemoryBarrier createImageMemoryBarrier(
    const BaseImage& image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    const uint32_t baseMipLevel = 0, const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0,
    const uint32_t layerCount = 1);
[[nodiscard]] VkImageMemoryBarrier2 createImageMemoryBarrier(
    const BaseImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    const uint32_t baseMipLevel = 0, const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0,
    const uint32_t layerCount = 1);
[[nodiscard]] VkImageMemoryBarrier2 createImageMemoryBarrier(
    const BaseImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex,
    const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, const uint32_t baseMipLevel = 0,
    const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0, const uint32_t layerCount = 1);

[[nodiscard]] VkImageMemoryBarrier createImageMemoryBarrier(
    const VkImage image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout,
    const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, const uint32_t baseMipLevel = 0,
    const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0, const uint32_t layerCount = 1);
[[nodiscard]] VkImageMemoryBarrier createImageMemoryBarrier(
    const VkImage image, const VkAccessFlags srcMask, const VkAccessFlags dstMask,
    const VkImageLayout oldLayout, const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex,
    const uint32_t dstQueueFamilyIndex, const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    const uint32_t baseMipLevel = 0, const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0,
    const uint32_t layerCount = 1);
[[nodiscard]] VkImageMemoryBarrier2 createImageMemoryBarrier(
    const VkImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    const uint32_t baseMipLevel = 0, const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0,
    const uint32_t layerCount = 1);
[[nodiscard]] VkImageMemoryBarrier2 createImageMemoryBarrier(
    const VkImage& image, const VkPipelineStageFlags2 srcStages, const VkAccessFlags2 srcMask,
    const VkPipelineStageFlags2 dstStages, const VkAccessFlags2 dstMask, const VkImageLayout oldLayout,
    const VkImageLayout newLayout, const uint32_t srcQueueFamilyIndex, const uint32_t dstQueueFamilyIndex,
    const VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT, const uint32_t baseMipLevel = 0,
    const uint32_t levelCount = 1, const uint32_t baseArrayLayer = 0, const uint32_t layerCount = 1);
} // namespace vkw
