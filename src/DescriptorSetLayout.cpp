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

#include "vkWrappers/wrappers/DescriptorSetLayout.hpp"

#include "vkWrappers/wrappers/utils.hpp"

namespace vkw
{
DescriptorSetLayout::DescriptorSetLayout(Device& device)
{
    VKW_CHECK_BOOL_THROW(this->init(device), "Initializing DescriptorSetLayout");
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& cp)
{
    this->clear();

    std::swap(device_, cp.device_);
    std::swap(descriptorSetLayout_, cp.descriptorSetLayout_);

    std::swap(bindings_, cp.bindings_);
    std::swap(storageBufferBindingCount_, cp.storageBufferBindingCount_);
    std::swap(uniformBufferBindingCount_, cp.uniformBufferBindingCount_);
    std::swap(storageImageBindingCount_, cp.combinedImageSamplerBindingCount_);
    std::swap(combinedImageSamplerBindingCount_, cp.combinedImageSamplerBindingCount_);

    std::swap(initialized_, cp.initialized_);

    return *this;
}

bool DescriptorSetLayout::init(Device& device)
{
    if(!initialized_)
    {
        device_ = &device;
        descriptorSetLayout_ = VK_NULL_HANDLE;

        storageBufferBindingCount_ = 0;
        uniformBufferBindingCount_ = 0;
        storageImageBindingCount_ = 0;
        combinedImageSamplerBindingCount_ = 0;

        initialized_ = true;
    }

    return true;
}

void DescriptorSetLayout::clear()
{
    bindings_.clear();

    VKW_DELETE_VK(DescriptorSetLayout, descriptorSetLayout_);

    storageBufferBindingCount_ = 0;
    uniformBufferBindingCount_ = 0;
    storageImageBindingCount_ = 0;
    combinedImageSamplerBindingCount_ = 0;

    device_ = nullptr;
    initialized_ = false;
}

DescriptorSetLayout& DescriptorSetLayout::addStorageBufferBinding(
    VkShaderStageFlags flags, uint32_t bindingPoint)
{
    if(descriptorSetLayout_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Cannot add binding: descriptor layout already created");
    }

    VkDescriptorSetLayoutBinding binding;
    binding.binding = bindingPoint;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = flags;
    binding.pImmutableSamplers = nullptr; // See what to do with this

    bindings_.push_back(binding);
    storageBufferBindingCount_++;
    return *this;
}

DescriptorSetLayout& DescriptorSetLayout::addUniformBufferBinding(
    VkShaderStageFlags flags, uint32_t bindingPoint)
{
    if(descriptorSetLayout_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Cannot add binding: descriptor layout already created");
    }

    VkDescriptorSetLayoutBinding binding;
    binding.binding = bindingPoint;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = flags;
    binding.pImmutableSamplers = nullptr; // See what to do with this

    bindings_.push_back(binding);
    uniformBufferBindingCount_++;
    return *this;
}

DescriptorSetLayout& DescriptorSetLayout::addStorageImageBinding(
    VkShaderStageFlags flags, uint32_t bindingPoint)
{
    if(descriptorSetLayout_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Cannot add binding: descriptor layout already created");
    }

    VkDescriptorSetLayoutBinding binding;
    binding.binding = bindingPoint;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    binding.descriptorCount = 1;
    binding.stageFlags = flags;
    binding.pImmutableSamplers = nullptr; // See what to do with this

    bindings_.push_back(binding);
    storageImageBindingCount_++;
    return *this;
}

DescriptorSetLayout& DescriptorSetLayout::addSamplerImageBinding(
    VkShaderStageFlags flags, uint32_t bindingPoint)
{
    if(descriptorSetLayout_ != VK_NULL_HANDLE)
    {
        throw std::runtime_error("Cannot add binding: descriptor layout already created");
    }

    VkDescriptorSetLayoutBinding binding;
    binding.binding = bindingPoint;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = flags;
    binding.pImmutableSamplers = nullptr; // See what to do with this

    bindings_.push_back(binding);
    combinedImageSamplerBindingCount_++;
    return *this;
}

void DescriptorSetLayout::create()
{
    VkDescriptorSetLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = static_cast<uint32_t>(bindings_.size());
    createInfo.pBindings = reinterpret_cast<const VkDescriptorSetLayoutBinding*>(bindings_.data());

    VKW_CHECK_VK_THROW(
        vkCreateDescriptorSetLayout(
            device_->getHandle(), &createInfo, nullptr, &descriptorSetLayout_),
        "Creating descriptor set layout");
}
} // namespace vkw
