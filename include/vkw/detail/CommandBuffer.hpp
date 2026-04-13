/*
 * Copyright (c) 2026 Adrien ARNAUD
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
    CommandBuffer(CommandBuffer&& rhs) { *this = std::move(rhs); }

    CommandBuffer& operator=(const CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&& rhs);
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

    const CommandBuffer& copyBuffer(
        const BaseBuffer& src, const BaseBuffer& dst, const std::span<VkBufferCopy>& regions) const;
    const CommandBuffer& copyBuffer(const BaseBuffer& src, const BaseBuffer& dst) const;

    const CommandBuffer& fillBuffer(
        const BaseBuffer& buffer, const uint32_t val, const size_t offset, const size_t size) const;

    const CommandBuffer& copyBufferToImage(
        const BaseBuffer& buffer, const BaseImage& image, const VkImageLayout dstLayout,
        const VkBufferImageCopy& region) const;
    const CommandBuffer& copyBufferToImage(
        const BaseBuffer& buffer, const BaseImage& image, VkImageLayout dstLayout,
        const std::span<VkBufferImageCopy>& regions) const;

    const CommandBuffer& copyImageToBuffer(
        const BaseImage& image, VkImageLayout srcLayout, const BaseBuffer& buffer,
        const VkBufferImageCopy& region) const;
    const CommandBuffer& copyImageToBuffer(
        const BaseImage& image, VkImageLayout srcLayout, const BaseBuffer& buffer,
        const std::span<VkBufferImageCopy>& regions) const;

    const CommandBuffer& blitImage(
        const BaseImage& src, const VkImageLayout srcLayout, const BaseImage& dst,
        const VkImageLayout dstLayout, const VkImageBlit region,
        const VkFilter filter = VK_FILTER_LINEAR) const;
    const CommandBuffer& blitImage(
        const VkImage src, const VkImageLayout srcLayout, const VkImage dst, const VkImageLayout dstLayout,
        const VkImageBlit region, const VkFilter filter = VK_FILTER_LINEAR) const;
    const CommandBuffer& blitImage(
        const BaseImage& src, const VkImageLayout srcLayout, const BaseImage& dst,
        const VkImageLayout dstLayout, const std::span<VkImageBlit>& regions,
        const VkFilter filter = VK_FILTER_LINEAR) const;
    const CommandBuffer& blitImage(
        const VkImage src, const VkImageLayout srcLayout, const VkImage dst, const VkImageLayout dstLayout,
        const std::span<VkImageBlit>& regions, const VkFilter filter = VK_FILTER_LINEAR) const;

    // -------------------------------------------------------------------------------------------------------
    // ----------------------------------- Pipeline barriers -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& memoryBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const VkMemoryBarrier& barrier) const;
    const CommandBuffer& memoryBarrier(const VkDependencyFlags flags, const VkMemoryBarrier2& barrier) const;
    const CommandBuffer& memoryBarrier(const VkMemoryBarrier2& barrier) const
    {
        return memoryBarrier({}, barrier);
    }

    const CommandBuffer& memoryBarriers(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkMemoryBarrier>& barriers) const;
    const CommandBuffer& memoryBarriers(
        const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& barriers) const;
    const CommandBuffer& memoryBarriers(const std::span<VkMemoryBarrier2>& barriers) const
    {
        return memoryBarriers({}, barriers);
    }

    const CommandBuffer& bufferMemoryBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const VkBufferMemoryBarrier& barrier) const;
    const CommandBuffer& bufferMemoryBarrier(
        const VkDependencyFlags flags, const VkBufferMemoryBarrier2& barrier) const;
    const CommandBuffer& bufferMemoryBarrier(const VkBufferMemoryBarrier2& barrier) const
    {
        return bufferMemoryBarrier({}, barrier);
    }

    const CommandBuffer& bufferMemoryBarriers(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkBufferMemoryBarrier>& barriers) const;
    const CommandBuffer& bufferMemoryBarriers(
        const VkDependencyFlags flags, const std::span<VkBufferMemoryBarrier2>& barriers) const;
    const CommandBuffer& bufferMemoryBarrier(const std::span<VkBufferMemoryBarrier2>& barriers) const
    {
        return bufferMemoryBarriers({}, barriers);
    }

    const CommandBuffer& imageMemoryBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const VkImageMemoryBarrier& barrier) const;
    const CommandBuffer& imageMemoryBarrier(
        const VkDependencyFlags flags, const VkImageMemoryBarrier2& barrier) const;
    const CommandBuffer& imageMemoryBarrier(const VkImageMemoryBarrier2& barrier) const
    {
        return imageMemoryBarrier({}, barrier);
    }

    const CommandBuffer& imageMemoryBarriers(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkImageMemoryBarrier>& barriers) const;
    const CommandBuffer& imageMemoryBarriers(
        const VkDependencyFlags flags, const std::span<VkImageMemoryBarrier2>& barriers) const;
    const CommandBuffer& imageMemoryBarriers(const std::span<VkImageMemoryBarrier2>& barriers) const
    {
        return imageMemoryBarriers({}, barriers);
    }

    const CommandBuffer& pipelineBarrier(
        const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkMemoryBarrier>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier>& imageMemoryBarriers) const;
    const CommandBuffer& pipelineBarrier(
        const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers) const;
    const CommandBuffer& pipelineBarrier(
        const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers) const
    {
        return pipelineBarrier({}, memoryBarriers, bufferMemoryBarriers, imageMemoryBarriers);
    }

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------------ Events -----------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& setEvent(const Event& event, const VkPipelineStageFlags flags) const;
    const CommandBuffer& setEvent(
        const Event& event, const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers) const;

    const CommandBuffer& waitEvent(
        const Event& event, const VkPipelineStageFlags srcFlags, const VkPipelineStageFlags dstFlags,
        const std::span<VkMemoryBarrier>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier>& imageMemoryBarriers) const;
    const CommandBuffer& waitEvent(
        const Event& event, const VkDependencyFlags flags, const std::span<VkMemoryBarrier2>& memoryBarriers,
        const std::span<VkBufferMemoryBarrier2>& bufferMemoryBarriers,
        const std::span<VkImageMemoryBarrier2>& imageMemoryBarriers) const;

    // -------------------------------------------------------------------------------------------------------
    // ----------------------------- Compute pipelines -------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& bindComputePipeline(const ComputePipeline& pipeline) const;

    const CommandBuffer& bindComputeDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const DescriptorSet& descriptorSet) const;
    const CommandBuffer& bindComputeDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const VkDescriptorSet descriptorSet) const;

    const CommandBuffer& bindComputeDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::vector<std::reference_wrapper<DescriptorSet>>& descriptorSets) const;
    const CommandBuffer& bindComputeDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::span<VkDescriptorSet>& descriptorSets) const;

    template <typename T>
    const CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout, const T& values, const ShaderStage stage) const
    {
        return pushConstants(pipelineLayout, &values, sizeof(T), stage);
    }
    const CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout, const void* values, const uint32_t size,
        const ShaderStage stage) const;

    template <typename T, typename... Args>
    const CommandBuffer& pushConstants(
        const PipelineLayout& pipelineLayout, const ShaderStage stage, const T& values,
        Args&&... stages) const
    {
        pushConstants(pipelineLayout, values, stage);
        return pushConstants(pipelineLayout, values, std::forward<Args>(stages)...);
    }

    const CommandBuffer& dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) const;
    const CommandBuffer& dispatchIndirect(
        const BaseBuffer& dispatchBuffer, const VkDeviceSize offset = 0) const;

    const CommandBuffer& pushComputeSamplerDescriptor(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler) const;

    const CommandBuffer& pushComputeCombinedImageSampler(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler, const VkImageView imageView, const VkImageLayout layout) const;

    const CommandBuffer& pushComputeSampledImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout) const;

    const CommandBuffer& pushComputeStorageImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout) const;

    const CommandBuffer& pushComputeUniformTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView) const;

    const CommandBuffer& pushComputeStorageTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView) const;

    const CommandBuffer& pushComputeStorageBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushComputeUniformBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushComputeStorageBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushComputeUniformBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushComputeAccelerationStructure(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkAccelerationStructureKHR accelerationStructure) const;

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------- Graphics pipeline -----------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& beginRenderPass(
        const RenderPass& renderPass, const VkFramebuffer frameBuffer, const VkOffset2D& offset,
        const VkExtent2D& extent, const VkClearColorValue& clearColor,
        const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    const CommandBuffer& nextSubpass(const VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
    const CommandBuffer& endRenderPass() const;

    const CommandBuffer& beginRendering(
        const RenderingAttachment& colorAttachment, const VkRect2D renderArea, const uint32_t viewMask = 0,
        const uint32_t layerCount = 1, const VkRenderingFlags flags = 0) const;
    const CommandBuffer& beginRendering(
        const std::span<RenderingAttachment>& colorAttachments, const VkRect2D renderArea,
        const uint32_t viewMask = 0, const uint32_t layerCount = 1, const VkRenderingFlags flags = 0) const;
    const CommandBuffer& beginRendering(
        RenderingAttachment& colorAttachment, RenderingAttachment& depthStencilAttachment,
        const VkRect2D renderArea, const uint32_t viewMask = 0, const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0) const;
    const CommandBuffer& beginRendering(
        const std::span<RenderingAttachment>& colorAttachments, RenderingAttachment& depthStencilAttachment,
        const VkRect2D renderArea, const uint32_t viewMask = 0, const uint32_t layerCount = 1,
        const VkRenderingFlags flags = 0) const;

    const CommandBuffer& endRendering() const;

    const CommandBuffer& bindGraphicsPipeline(GraphicsPipeline& pipeline) const;

    const CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const DescriptorSet& descriptorSet) const;
    const CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const VkDescriptorSet descriptorSet) const;

    const CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::vector<std::reference_wrapper<DescriptorSet>>& descriptorSets) const;
    const CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const DescriptorSet& descriptorSet) const
    {
        return bindGraphicsDescriptorSet(pipelineLayout, firstSet, descriptorSet);
    }
    template <typename... Args>
    const CommandBuffer& bindGraphicsDescriptorSet(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet, const DescriptorSet& descriptorSet,
        Args&&... args) const
    {
        bindGraphicsDescriptorSets(pipelineLayout, firstSet, descriptorSet);
        return bindGraphicsDescriptorSets(pipelineLayout, firstSet + 1, std::forward<Args>(args)...);
    }
    const CommandBuffer& bindGraphicsDescriptorSets(
        const PipelineLayout& pipelineLayout, const uint32_t firstSet,
        const std::span<VkDescriptorSet>& descriptorSets) const;

    const CommandBuffer& pushGraphicsSamplerDescriptor(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler) const;

    const CommandBuffer& pushGraphicsCombinedImageSampler(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkSampler sampler, const VkImageView imageView, const VkImageLayout layout) const;

    const CommandBuffer& pushGraphicsSampledImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout) const;

    const CommandBuffer& pushGraphicsStorageImage(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkImageView imageView, const VkImageLayout layout) const;

    const CommandBuffer& pushGraphicsUniformTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView) const;

    const CommandBuffer& pushGraphicsStorageTexelBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBufferView& bufferView) const;

    const CommandBuffer& pushGraphicsStorageBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushGraphicsUniformBuffer(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushGraphicsStorageBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushGraphicsUniformBufferDynamic(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkBuffer buffer, const VkDeviceSize offset, const VkDeviceSize range) const;

    const CommandBuffer& pushGraphicsAccelerationStructure(
        const PipelineLayout& pipelineLayout, const uint32_t set, const uint32_t binding,
        const VkAccelerationStructureKHR accelerationStructure) const;

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------------- Dynamic states -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& setViewport(
        const float offX, const float offY, const float width, const float height,
        const float minDepth = 0.0f, const float maxDepth = 1.0f) const;
    const CommandBuffer& setViewport(const VkViewport& viewport) const;
    const CommandBuffer& setViewport(const std::span<VkViewport>& viewports, const uint32_t offset = 0) const;

    const CommandBuffer& setScissor(const VkOffset2D& offset, const VkExtent2D& extent) const;
    const CommandBuffer& setScissor(const VkRect2D& scissor) const;
    const CommandBuffer& setScissor(const std::span<VkRect2D>& scissors, const uint32_t offset = 0) const;

    const CommandBuffer& setLineWidth(const float lineWidth) const;

    const CommandBuffer& setDepthBias(
        const float depthBiasConstantFactor, const float depthBiasClamp,
        const float depthBiasSlopeFactor) const;

    const CommandBuffer& setBlendConstants(const float r, const float g, const float b, const float a) const;

    const CommandBuffer& setStencilCompareMask(
        const VkStencilFaceFlags faceMask, const uint32_t compareMask) const;

    const CommandBuffer& setStencilWriteMask(
        const VkStencilFaceFlags faceMask, const uint32_t writeMask) const;

    const CommandBuffer& setStencilReference(
        const VkStencilFaceFlags faceMask, const uint32_t reference) const;

    const CommandBuffer& setCullMode(const VkCullModeFlags cullMode) const;

    const CommandBuffer& setFrontFace(const VkFrontFace frontFace) const;

    const CommandBuffer& setPrimitiveTopology(const VkPrimitiveTopology topology) const;

    const CommandBuffer& setViewportWithCount(const std::span<VkViewport>& viewports) const;

    const CommandBuffer& setScissorWithCount(const std::span<VkRect2D>& scissors) const;

    const CommandBuffer& setDepthTestEnable(const VkBool32 depthTestEnable) const;

    const CommandBuffer& setDepthWriteEnable(const VkBool32 depthWriteEnable) const;

    const CommandBuffer& setDepthCompareOp(const VkCompareOp compareOp) const;

    const CommandBuffer& setDepthBoundsTestEnable(const VkBool32 depthBoundsTestEnable) const;

    const CommandBuffer& setStencilTestEnable(const VkBool32 stencilTestEnable) const;

    const CommandBuffer& setStencilOp(
        const VkStencilFaceFlags faceMask, const VkStencilOp failOp, const VkStencilOp passOp,
        const VkStencilOp depthFailOp, const VkCompareOp compareOp) const;

    const CommandBuffer& setDepthBiasEnable(const VkBool32 depthBiasEnable) const;

    // -------------------------------------------------------------------------------------------------------
    // ----------------------------------------- Drawing -----------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& bindVertexBuffer(
        const uint32_t binding, const BaseBuffer& buffer, const VkDeviceSize offset) const;
    const CommandBuffer& bindVertexBuffer(
        const uint32_t binding, const VkBuffer& buffer, const VkDeviceSize byteOffset) const;

    const CommandBuffer& bindVertexBuffers(
        const uint32_t firstBinding,
        const std::tuple<const BaseBuffer&, const VkDeviceSize>& bufferData) const
    {
        const auto& [buffer, size] = bufferData;
        return bindVertexBuffer(firstBinding, buffer, size);
    }
    template <typename... Args>
    const CommandBuffer& bindVertexBuffers(
        const uint32_t firstBinding, const std::tuple<const BaseBuffer&, const VkDeviceSize>& bufferData,
        Args&&... args) const
    {
        bindVertexBuffers(firstBinding, bufferData);
        return bindVertexBuffers(firstBinding + 1, std::forward<Args>(args)...);
    }
    const CommandBuffer& bindVertexBuffers(
        const uint32_t firstBinding, const std::span<VkBuffer>& buffers,
        const std::span<VkDeviceSize>& byteOffsets) const;

    const CommandBuffer& bindIndexBuffer(
        const BaseBuffer& buffer, const VkIndexType indexType, const VkDeviceSize byteOffset = 0) const;
    const CommandBuffer& bindIndexBuffer(
        const VkBuffer& buffer, const VkIndexType indexType, const VkDeviceSize byteOffset = 0) const;

    const CommandBuffer& draw(
        const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex,
        const uint32_t firstInstance) const;
    const CommandBuffer& drawIndirect(
        const BaseBuffer& indirectBuffer, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand)) const;
    const CommandBuffer& drawIndirect(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand)) const;
    const CommandBuffer& drawIndirectCount(
        const BaseBuffer& indirectBuffer, const BaseBuffer& countBuffer, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand)) const;
    const CommandBuffer& drawIndirectCount(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const BaseBuffer& countBuffer,
        const size_t countOffsetBytes, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndirectCommand)) const;

    const CommandBuffer& drawIndexed(
        const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
        const uint32_t vertexOffset, const uint32_t firstInstance) const;
    const CommandBuffer& drawIndexedIndirect(
        const BaseBuffer& indirectBuffer, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand)) const;
    const CommandBuffer& drawIndexedIndirect(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const uint32_t drawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand)) const;
    const CommandBuffer& drawIndexedIndirectCount(
        const BaseBuffer& indirectBuffer, const BaseBuffer& countBuffer, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand)) const;
    const CommandBuffer& drawIndexedIndirectCount(
        const BaseBuffer& indirectBuffer, const VkDeviceSize offsetBytes, const BaseBuffer& countBuffer,
        const size_t countOffsetBytes, const uint32_t maxDrawCount,
        const uint32_t stride = sizeof(VkDrawIndexedIndirectCommand)) const;

    const CommandBuffer& drawMeshTasks(
        const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) const;
    const CommandBuffer& drawMeshTasksIndirect(
        const BaseBuffer& buffer, const VkDeviceSize offset, const uint32_t drawCount,
        const uint32_t stride) const;
    const CommandBuffer& drawMeshTasksIndirectCount(
        const BaseBuffer& buffer, const VkDeviceSize offset, const BaseBuffer& countBuffer,
        const VkDeviceSize countBufferOffset, const uint32_t maxDrawCount, const uint32_t stride) const;

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------- Acceleration structures -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& buildAccelerationStructure(
        const BottomLevelAccelerationStructure& blas, const BaseBuffer& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {}) const;
    const CommandBuffer& buildAccelerationStructure(
        const TopLevelAccelerationStructure& tlas, const BaseBuffer& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {}) const;

    const CommandBuffer& updateAccelerationStructure(
        TopLevelAccelerationStructure& tlas, const BaseBuffer& scratchBuffer,
        const VkBuildAccelerationStructureFlagsKHR buildFlags = {}) const;
    const CommandBuffer& updateAccelerationStructure(
        TopLevelAccelerationStructure& tlas, const std::span<VkTransformMatrixKHR>& transforms,
        const BaseBuffer& scratchBuffer, const VkBuildAccelerationStructureFlagsKHR buildFlags = {}) const;

    ///@todo Implement buildAccelerationStructures()
    ///@todo Implement buildAccelerationStructureIndirect
    ///@todo Implement buildAccelerationStructuresIndirect()
    ///@todo Implement copyAccelerationStructure()

    // -------------------------------------------------------------------------------------------------------

    ///@todo Implement setRayTracingPipelineStackSize()
    ///@todo Implement traceRaysIndirect()
    ///@todo Implement traceRays()

    // -------------------------------------------------------------------------------------------------------
    // ------------------------------------ Debug utils-------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    const CommandBuffer& insertDebugMarker(const char* name, const float color[4]) const;

    const CommandBuffer& beginDebugRegion(const char* name, const float color[4]) const;

    const CommandBuffer& endDebugRegion() const;

    // -------------------------------------------------------------------------------------------------------
    VkCommandBuffer getHandle() const { return commandBuffer_; }

  private:
    const Device* device_{nullptr};
    VkCommandPool cmdPool_{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};

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
