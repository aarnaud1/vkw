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

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/BufferView.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/DescriptorPool.hpp"
#include "vkw/detail/DescriptorSetLayout.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/ImageView.hpp"
#include "vkw/detail/Sampler.hpp"
#include "vkw/detail/TopLevelAS.hpp"

#include <cstdlib>
#include <span>

namespace vkw
{
class DescriptorSet
{
  public:
    constexpr DescriptorSet() {}
    DescriptorSet(
        const Device& device, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool,
        const void* pCreateNext = nullptr);

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&& rhs) { *this = std::move(rhs); };

    DescriptorSet& operator=(const DescriptorSet&) = delete;
    DescriptorSet& operator=(DescriptorSet&& rhs);

    ~DescriptorSet();

    bool init(
        const Device& device, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool,
        const void* pCreateNext = nullptr);

    void clear();

    bool initialized() const { return initialized_; }

    VkDescriptorSet getHandle() const { return descriptorSet_; }

    // -------------------------------------------------------------------------------------------------------
    // ---------------------------------------- Sampler ------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindSampler(const uint32_t binding, const uint32_t index, const Sampler& sampler)
    {
        return bindSampler(binding, index, sampler.getHandle());
    }
    DescriptorSet& bindSampler(const uint32_t binding, const uint32_t index, const VkSampler sampler);

    DescriptorSet& bindSamplers(
        const uint32_t binding, const uint32_t index, std::vector<std::reference_wrapper<Sampler>>& sampler);
    DescriptorSet& bindSamplers(
        const uint32_t binding, const uint32_t index, const std::span<VkSampler>& samplers);

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Combined image sampler -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindCombinedImageSampler(
        const uint32_t binding, const uint32_t index, const Sampler& sampler, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindCombinedImageSampler(binding, index, sampler.getHandle(), imageView.getHandle(), layout);
    }
    DescriptorSet& bindCombinedImageSampler(
        const uint32_t binding, const uint32_t index, const VkSampler sampler, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindCombinedImageSamplers(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<Sampler>>& samplers,
        const std::vector<std::reference_wrapper<ImageView>>& imageViews,
        const std::span<VkImageLayout>& layouts = {});
    DescriptorSet& bindCombinedImageSamplers(
        const uint32_t binding, const uint32_t index, const std::span<VkSampler>& samplers,
        const std::span<VkImageView>& imageViews, const std::span<VkImageLayout>& layouts = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Sampled image --------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindSampledImage(
        const uint32_t binding, const uint32_t index, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindSampledImage(binding, index, imageView.getHandle(), layout);
    }
    DescriptorSet& bindSampledImage(
        const uint32_t binding, const uint32_t index, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindSampledImages(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<ImageView>>& imageViews,
        const std::span<VkImageLayout>& layouts = {});
    DescriptorSet& bindSampledImages(
        const uint32_t binding, const uint32_t index, const std::span<VkImageView>& imageViews,
        const std::span<VkImageLayout>& layouts = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage image --------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindStorageImage(
        const uint32_t binding, const uint32_t index, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return bindStorageImage(binding, index, imageView.getHandle(), layout);
    }
    DescriptorSet& bindStorageImage(
        const uint32_t binding, const uint32_t index, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

    DescriptorSet& bindStorageImages(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<ImageView>>& imageViews,
        const std::span<VkImageLayout>& layouts = {});
    DescriptorSet& bindStorageImages(
        const uint32_t binding, const uint32_t index, const std::span<VkImageView>& imageViews,
        const std::span<VkImageLayout>& layouts = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Uniform texel buffer -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindUniformTexelBuffer(
        const uint32_t binding, const uint32_t index, const BufferView& bufferView)
    {
        return bindUniformTexelBuffer(binding, index, bufferView.getHandle());
    }
    DescriptorSet& bindUniformTexelBuffer(
        const uint32_t binding, const uint32_t index, const VkBufferView& bufferView);

    DescriptorSet& bindUniformTexelBuffers(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<BufferView>>& bufferViews);
    DescriptorSet& bindUniformTexelBuffers(
        const uint32_t binding, const uint32_t index, const std::span<VkBufferView>& bufferViews);

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage texel buffer -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindStorageTexelBuffer(
        const uint32_t binding, const uint32_t index, const BufferView& bufferView)
    {
        return bindStorageTexelBuffer(binding, index, bufferView.getHandle());
    }
    DescriptorSet& bindStorageTexelBuffer(
        const uint32_t binding, const uint32_t index, const VkBufferView& bufferView);

    DescriptorSet& bindStorageTexelBuffers(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<BufferView>>& bufferViews);
    DescriptorSet& bindStorageTexelBuffers(
        const uint32_t binding, const uint32_t index, const std::span<VkBufferView>& bufferViews);

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Uniform buffer -------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindUniformBuffer(
        const uint32_t binding, const uint32_t index, const BaseBuffer& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * buffer.stride();
        return bindUniformBuffer(binding, index, buffer.getHandle(), offset * buffer.stride(), bufferRange);
    }
    DescriptorSet& bindUniformBuffer(
        const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindUniformBuffers(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<BaseBuffer>>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});
    DescriptorSet& bindUniformBuffers(
        const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage buffer -------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindStorageBuffer(
        const uint32_t binding, const uint32_t index, const BaseBuffer& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * buffer.stride();
        return bindStorageBuffer(binding, index, buffer.getHandle(), offset * buffer.stride(), bufferRange);
    }
    DescriptorSet& bindStorageBuffer(
        const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindStorageBuffers(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<BaseBuffer>>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});
    DescriptorSet& bindStorageBuffers(
        const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffer,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Unifrom buffer synamic -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindUniformBufferDynamic(
        const uint32_t binding, const uint32_t index, const BaseBuffer& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * buffer.stride();
        return bindUniformBufferDynamic(
            binding, index, buffer.getHandle(), offset * buffer.stride(), bufferRange);
    }
    DescriptorSet& bindUniformBufferDynamic(
        const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindUniformBuffersDynamic(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<BaseBuffer>>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});
    DescriptorSet& bindUniformBuffersDynamic(
        const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage buffer synamic -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindStorageBufferDynamic(
        const uint32_t binding, const uint32_t index, const BaseBuffer& buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE)
    {
        const auto bufferRange = (range == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : range * buffer.stride();
        return bindStorageBufferDynamic(
            binding, index, buffer.getHandle(), offset * buffer.stride(), bufferRange);
    }
    DescriptorSet& bindStorageBufferDynamic(
        const uint32_t binding, const uint32_t index, const VkBuffer buffer, const VkDeviceSize offset = 0,
        const VkDeviceSize range = VK_WHOLE_SIZE);

    DescriptorSet& bindStorageBuffersDynamic(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<BaseBuffer>>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});
    DescriptorSet& bindStorageBuffersDynamic(
        const uint32_t binding, const uint32_t index, const std::span<VkBuffer>& buffers,
        const std::span<VkDeviceSize>& offsets = {}, const std::span<VkDeviceSize>& ranges = {});

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Acceleration structure -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorSet& bindAccelerationStructure(
        const uint32_t binding, const uint32_t index,
        const TopLevelAccelerationStructure& accelerationStructure)
    {
        return bindAccelerationStructure(binding, index, accelerationStructure.getHandle());
    }
    DescriptorSet& bindAccelerationStructure(
        const uint32_t binding, const uint32_t index, const VkAccelerationStructureKHR accelerationStructure);

    DescriptorSet& bindAccelerationStructures(
        const uint32_t binding, const uint32_t index,
        const std::vector<std::reference_wrapper<TopLevelAccelerationStructure>>& accelerationStructures);
    DescriptorSet& bindAccelerationStructures(
        const uint32_t binding, const uint32_t index,
        const std::span<VkAccelerationStructureKHR>& accelerationStructures);

  private:
    const Device* device_{nullptr};
    const DescriptorPool* descriptorPool_{nullptr};

    VkDescriptorSet descriptorSet_{VK_NULL_HANDLE};

    bool initialized_{false};
};
} // namespace vkw