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

#include "vkw/detail/Buffer.hpp"
#include "vkw/detail/BufferView.hpp"
#include "vkw/detail/Common.hpp"
#include "vkw/detail/Device.hpp"
#include "vkw/detail/ImageView.hpp"
#include "vkw/detail/MemoryCommon.hpp"
#include "vkw/detail/Sampler.hpp"
#include "vkw/detail/TopLevelAS.hpp"
#include "vkw/detail/utils.hpp"

namespace vkw
{
template <MemoryType memType>
using BaseDescriptorBuffer = Buffer<
    uint8_t, memType,
    VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT>;

template <MemoryType memType>
class DescriptorBuffer final : public BaseDescriptorBuffer<memType>
{
  public:
    constexpr DescriptorBuffer() {}

    explicit DescriptorBuffer(
        const Device& device, const size_t size, const VkBufferUsageFlags usage = {},
        const VkDeviceSize alignment = 0, const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {}, void* pCreateNext = nullptr,
        const char* pName = nullptr)
        : BaseDescriptorBuffer<memType>(
              device, size, usage, alignment, sharingMode, queueFamilyIndices, pCreateNext, pName)
    {}

    explicit DescriptorBuffer(
        const Device& device, const VkBufferCreateInfo& createInfo, const VkDeviceSize alignment = 0,
        const char* pName = nullptr)
        : BaseDescriptorBuffer<memType>(device, createInfo, alignment, pName)
    {}

    DescriptorBuffer(const DescriptorBuffer&) = delete;
    DescriptorBuffer(DescriptorBuffer&& rhs) : BaseDescriptorBuffer<memType>(std::move(rhs)) {}

    DescriptorBuffer& operator=(const DescriptorBuffer&) = delete;
    DescriptorBuffer& operator=(DescriptorBuffer&& rhs)
    {
        BaseDescriptorBuffer<memType>::operator=(std::move(rhs));
        return *this;
    }

    ~DescriptorBuffer() = default;

    // -------------------------------------------------------------------------------------------------------
    // ---------------------------------------- Sampler ------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeSampler(const VkDeviceSize offset, const Sampler& sampler)
    {
        return writeSampler(offset, sampler.getHandle());
    }

    DescriptorBuffer& writeSampler(const VkDeviceSize offset, const VkSampler sampler)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLER;
        getInfo.data.pSampler = &sampler;

        const uint32_t descriptorSize = this->device_->getDescriptorBufferProperties().samplerDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Combined image sampler -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeCombinedImageSampler(
        const VkDeviceSize offset, const Sampler& sampler, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return writeCombinedImageSampler(offset, sampler.getHandle(), imageView.getHandle(), layout);
    }

    DescriptorBuffer& writeCombinedImageSampler(
        const VkDeviceSize offset, const VkSampler sampler, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        const VkDescriptorImageInfo imgInfo = {sampler, imageView, layout};

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        getInfo.data.pCombinedImageSampler = &imgInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().combinedImageSamplerDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Sampled image --------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeSampledImage(
        const VkDeviceSize offset, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return writeSampledImage(offset, imageView.getHandle(), layout);
    }

    DescriptorBuffer& writeSampledImage(
        const VkDeviceSize offset, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        getInfo.data.pSampledImage = &imgInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().sampledImageDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage image --------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeStorageImage(
        const VkDeviceSize offset, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return writeStorageImage(offset, imageView.getHandle(), layout);
    }

    DescriptorBuffer& writeStorageImage(
        const VkDeviceSize offset, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        getInfo.data.pStorageImage = &imgInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().storageImageDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Uniform texel buffer -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeUniformTexelBuffer(
        const VkDeviceSize offset, const BaseBuffer& buffer, const VkFormat format,
        const VkDeviceSize bufferOffset = 0, const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        const auto bufferAddress = buffer.deviceAddress() + bufferOffset;
        const auto bufferRangeBytes
            = (bufferRange == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : bufferRange * buffer.stride();
        return writeUniformTexelBuffer(offset, bufferAddress, format, bufferRangeBytes);
    }

    DescriptorBuffer& writeUniformTexelBuffer(
        const VkDeviceSize offset, const VkDeviceAddress bufferAddress, const VkFormat format,
        const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        VkDescriptorAddressInfoEXT bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
        bufferInfo.pNext = nullptr;
        bufferInfo.address = bufferAddress;
        bufferInfo.format = format;
        bufferInfo.range = bufferRange;

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        getInfo.data.pUniformTexelBuffer = &bufferInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().uniformTexelBufferDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage texel buffer -------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeStorageTexelBuffer(
        const VkDeviceSize offset, const BaseBuffer& buffer, const VkFormat format,
        const VkDeviceSize bufferOffset = 0, const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        const auto bufferAddress = buffer.deviceAddress() + bufferOffset;
        const auto bufferRangeBytes
            = (bufferRange == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : bufferRange * buffer.stride();
        return writeStorageTexelBuffer(offset, bufferAddress, format, bufferRangeBytes);
    }

    DescriptorBuffer& writeStorageTexelBuffer(
        const VkDeviceSize offset, const VkDeviceAddress bufferAddress, const VkFormat format,
        const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        VkDescriptorAddressInfoEXT bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
        bufferInfo.pNext = nullptr;
        bufferInfo.address = bufferAddress;
        bufferInfo.format = format;
        bufferInfo.range = bufferRange;

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        getInfo.data.pStorageTexelBuffer = &bufferInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().storageTexelBufferDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Uniform buffer -------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeUniformBuffer(
        const VkDeviceSize offset, const BaseBuffer& buffer, const VkFormat format,
        const VkDeviceSize bufferOffset = 0, const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        const auto bufferAddress = buffer.deviceAddress() + bufferOffset;
        const auto bufferRangeBytes
            = (bufferRange == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : bufferRange * buffer.stride();
        return writeUniformBuffer(offset, bufferAddress, format, bufferRangeBytes);
    }

    DescriptorBuffer& writeUniformBuffer(
        const VkDeviceSize offset, const VkDeviceAddress bufferAddress, const VkFormat format,
        const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        VkDescriptorAddressInfoEXT bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
        bufferInfo.pNext = nullptr;
        bufferInfo.address = bufferAddress;
        bufferInfo.format = format;
        bufferInfo.range = bufferRange;

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        getInfo.data.pUniformBuffer = &bufferInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().uniformBufferDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Storage buffer -------------------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeStorageBuffer(
        const VkDeviceSize offset, const BaseBuffer& buffer, const VkFormat format,
        const VkDeviceSize bufferOffset = 0, const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        const auto bufferAddress = buffer.deviceAddress() + bufferOffset;
        const auto bufferRangeBytes
            = (bufferRange == VK_WHOLE_SIZE) ? VK_WHOLE_SIZE : bufferRange * buffer.stride();
        return writeStorageBuffer(offset, bufferAddress, format, bufferRangeBytes);
    }

    DescriptorBuffer& writeStorageBuffer(
        const VkDeviceSize offset, const VkDeviceAddress bufferAddress, const VkFormat format,
        const VkDeviceSize bufferRange = VK_WHOLE_SIZE)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        VkDescriptorAddressInfoEXT bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
        bufferInfo.pNext = nullptr;
        bufferInfo.address = bufferAddress;
        bufferInfo.format = format;
        bufferInfo.range = bufferRange;

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        getInfo.data.pStorageBuffer = &bufferInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().storageBufferDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------------- Input attachment -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeInputAttachment(
        const VkDeviceSize offset, const ImageView& imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        return writeInputAttachment(offset, imageView.getHandle(), layout);
    }

    DescriptorBuffer& writeInputAttachment(
        const VkDeviceSize offset, const VkImageView imageView,
        const VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        const VkDescriptorImageInfo imgInfo = {VK_NULL_HANDLE, imageView, layout};

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        getInfo.data.pInputAttachmentImage = &imgInfo;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().inputAttachmentDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }

    // -------------------------------------------------------------------------------------------------------
    // -------------------------------- Acceleration structure -----------------------------------------------
    // -------------------------------------------------------------------------------------------------------

    DescriptorBuffer& writeAccelerationStructure(
        const VkDeviceSize& offset, const TopLevelAccelerationStructure& accelerationStructure)
    {
        return writeAccelerationStructure(offset, accelerationStructure.getDeviceAddress());
    }

    DescriptorBuffer& writeAccelerationStructure(
        const VkDeviceSize offset, const VkDeviceAddress accelerationStructureAddress)
    {
        static_assert(
            (memType == MemoryType::Host) || (memType == MemoryType::HostStaging),
            "Writting from CPU require random accessed descriptor buffer type");

        VkDescriptorGetInfoEXT getInfo = {};
        getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
        getInfo.pNext = nullptr;
        getInfo.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        getInfo.data.accelerationStructure = accelerationStructureAddress;

        const uint32_t descriptorSize
            = this->device_->getDescriptorBufferProperties().accelerationStructureDescriptorSize;
        this->device_->vk().vkGetDescriptorEXT(
            this->device_->getHandle(), &getInfo, descriptorSize, static_cast<void*>(this->data_ + offset));

        return *this;
    }
};
} // namespace vkw