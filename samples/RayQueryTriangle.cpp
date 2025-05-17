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

#include "RayQueryTriangle.hpp"

#include <cstdlib>

RayQueryTriangle::RayQueryTriangle()
{
    // Ray query features
    rayQueryFeatures_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    rayQueryFeatures_.pNext = nullptr;
    rayQueryFeatures_.rayQuery = VK_TRUE;

    // Acceleration structure features
    deviceAccelerationStructureFeatures_.sType
        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    deviceAccelerationStructureFeatures_.pNext = &rayQueryFeatures_;
    deviceAccelerationStructureFeatures_.accelerationStructure = VK_TRUE;
    deviceAccelerationStructureFeatures_.accelerationStructureCaptureReplay = VK_FALSE;
    deviceAccelerationStructureFeatures_.accelerationStructureHostCommands = VK_FALSE;
    deviceAccelerationStructureFeatures_.accelerationStructureIndirectBuild = VK_FALSE;
    deviceAccelerationStructureFeatures_.descriptorBindingAccelerationStructureUpdateAfterBind
        = VK_FALSE;

    // Device address features
    deviceAddressFeatures_.sType
        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
    deviceAddressFeatures_.pNext = &deviceAccelerationStructureFeatures_;
    deviceAddressFeatures_.bufferDeviceAddressCaptureReplay = VK_FALSE;
    deviceAddressFeatures_.bufferDeviceAddressMultiDevice = VK_FALSE;
    deviceAddressFeatures_.bufferDeviceAddress = VK_TRUE;

    // Add features
    deviceFeatures_.pNext = &deviceAddressFeatures_;

    // Add required extensions
    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
}

VkPhysicalDevice RayQueryTriangle::findSupportedDevice() const
{
    return findCompatibleDevice(instance_, deviceExtensions_, deviceAddressFeatures_);
}

bool RayQueryTriangle::init()
{
    VKW_CHECK_BOOL_RETURN_FALSE(descriptorSetLayout_.init(device_));
    descriptorSetLayout_
        .addBinding<vkw::DescriptorType::AccelerationStructure>(VK_SHADER_STAGE_COMPUTE_BIT, 0)
        .addBinding<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_COMPUTE_BIT, 1)
        .create();

    VKW_CHECK_BOOL_RETURN_FALSE(pipelineLayout_.init(device_, descriptorSetLayout_));
    pipelineLayout_.reservePushConstants<PushConstants>(vkw::ShaderStage::Compute);
    pipelineLayout_.create();

    VKW_CHECK_BOOL_RETURN_FALSE(pipeline_.init(device_, "build/spv/ray_query_triangle.comp.spv"));
    pipeline_.createPipeline(pipelineLayout_);

    // Init buffers
    VKW_CHECK_BOOL_RETURN_FALSE(vertexBuffer_.init(
        device_,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        vertexCount));
    VKW_CHECK_BOOL_RETURN_FALSE(indexBuffer_.init(
        device_,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        3 * triangleCount));
    VKW_CHECK_BOOL_RETURN_FALSE(transformBuffer_.init(
        device_,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        1));

    uploadData(device_, triangleData, vertexBuffer_);
    uploadData(device_, indices, indexBuffer_);
    uploadData(device_, &transform, transformBuffer_);

    // Build acceleration structures
    geometryData_
        = GeometryType{vertexBuffer_, indexBuffer_, transformBuffer_, 3, sizeof(glm::vec3), 1};
    VKW_CHECK_BOOL_RETURN_FALSE(bottomLevelAs_.init(device_));
    bottomLevelAs_.addGeometry(geometryData_, VK_GEOMETRY_OPAQUE_BIT_KHR)
        .create(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

    VKW_CHECK_BOOL_RETURN_FALSE(topLevelAs_.init(device_));
    topLevelAs_.addInstance(bottomLevelAs_)
        .create(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

    VkPhysicalDeviceAccelerationStructurePropertiesKHR asProperties = {};
    asProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
    asProperties.pNext = nullptr;

    VkPhysicalDeviceProperties2 properties = {};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties.pNext = &asProperties;

    vkGetPhysicalDeviceProperties2(device_.getPhysicalDevice(), &properties);

    VKW_CHECK_BOOL_RETURN_FALSE(scratchBuffer_.init(
        device_,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        std::max(bottomLevelAs_.buildScratchSize(), topLevelAs_.buildScratchSize()),
        asProperties.minAccelerationStructureScratchOffsetAlignment));

    outputImages_.resize(framesInFlight);
    outputImagesViews_.resize(framesInFlight);
    for(uint32_t i = 0; i < framesInFlight; ++i)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(outputImages_[i].init(
            device_,
            VK_IMAGE_TYPE_2D,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VkExtent3D{initWidth, initHeight, 1},
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));

        VKW_CHECK_BOOL_RETURN_FALSE(outputImagesViews_[i].init(
            device_,
            outputImages_[i],
            VK_IMAGE_VIEW_TYPE_2D,
            VK_FORMAT_R32G32B32A32_SFLOAT,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}));
    }

    descriptorPool_.init(
        device_,
        framesInFlight,
        {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, framesInFlight},
         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, framesInFlight}});
    descriptorSets_ = descriptorPool_.allocateDescriptorSets(descriptorSetLayout_, framesInFlight);

    for(uint32_t i = 0; i < framesInFlight; ++i)
    {
        descriptorSets_[i].bindAccelerationStructure(0, topLevelAs_);
        descriptorSets_[i].bindStorageImage(1, outputImagesViews_[i]);
    }

    return true;
}

bool RayQueryTriangle::recordInitCommands(vkw::CommandBuffer& initCmdBuffer, const uint32_t frameId)
{
    initCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    initCmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        vkw::createImageMemoryBarrier(
            outputImages_[frameId],
            0,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL));
    if(frameId == 0)
    {
        initCmdBuffer.buildAccelerationStructure(bottomLevelAs_, scratchBuffer_);
        initCmdBuffer.bufferMemoryBarrier(
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            vkw::createBufferMemoryBarrier(
                scratchBuffer_,
                VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR));
        initCmdBuffer.buildAccelerationStructure(topLevelAs_, scratchBuffer_);
    }
    initCmdBuffer.end();
    return true;
}

void RayQueryTriangle::recordDrawCommands(
    vkw::CommandBuffer& cmdBuffer, const uint32_t frameId, const uint32_t imageId)
{
    const uint32_t workGroupCountX = vkw::utils::divUp(initWidth, 16);
    const uint32_t workGroupCountY = vkw::utils::divUp(initHeight, 16);

    struct PushConstants params{};
    params.sizeX = initWidth;
    params.sizeY = initHeight;

    cmdBuffer.reset();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    cmdBuffer.bindComputePipeline(pipeline_);
    cmdBuffer.bindComputeDescriptorSet(pipelineLayout_, 0, descriptorSets_[frameId]);
    cmdBuffer.pushConstants(pipelineLayout_, params, vkw::ShaderStage::Compute);
    cmdBuffer.dispatch(workGroupCountX, workGroupCountY);

    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createImageMemoryBarrier(
            outputImages_[frameId],
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_GENERAL));
    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        vkw::createImageMemoryBarrier(
            swapchain_.images()[imageId],
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

    // Perform blit operation
    VkImageBlit region = {};
    region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.srcOffsets[0] = {0, 0, 0};
    region.srcOffsets[1] = {initWidth, initHeight, 1};
    region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.dstOffsets[0] = {0, 0, 0};
    region.dstOffsets[1]
        = {static_cast<int32_t>(frameWidth_), static_cast<int32_t>(frameHeight_), 1};
    cmdBuffer.blitImage(
        outputImages_[frameId].getHandle(),
        VK_IMAGE_LAYOUT_GENERAL,
        swapchain_.images()[imageId],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        region);

    cmdBuffer.imageMemoryBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        vkw::createImageMemoryBarrier(
            swapchain_.images()[imageId],
            VK_ACCESS_TRANSFER_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
    cmdBuffer.end();
}

bool RayQueryTriangle::recordPostDrawCommands(
    vkw::CommandBuffer& /*cmdBuffer*/, const uint32_t /*frameId*/, const uint32_t /*imageId*/)
{
    return false;
}

bool RayQueryTriangle::postDraw() { return true; }