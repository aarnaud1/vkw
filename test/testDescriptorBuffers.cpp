#include "utils.hpp"

#include <cstdio>
#include <cstdlib>
#include <vkw/high_level/Types.hpp>
#include <vkw/vkw.hpp>

static bool testStorageImageDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize);

// -----------------------------------------------------------------------------------------------------------------------------

static const uint32_t fillStorageImagesDescriptorIndexingComp[] = {
#include "fillStorageImagesDescriptorIndexing.comp.spv"
};

// -----------------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};

    vkw::Instance instance{};
    check_exit(instance.init(instanceLayers, {}));

    const std::vector<const char*> requiredExtensions = {VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME};

    VkPhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeatures = {};
    descriptorBufferFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    descriptorBufferFeatures.pNext = nullptr;
    descriptorBufferFeatures.descriptorBuffer = VK_TRUE;
    descriptorBufferFeatures.descriptorBufferPushDescriptors = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {};
    descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptorIndexingFeatures.pNext = &descriptorBufferFeatures;
    descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    descriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;

    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
    bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceAddressFeatures.pNext = &descriptorIndexingFeatures;
    bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

    const auto supportedDevices = vkw::Device::listSupportedDevices(
        instance,
        requiredExtensions,
        {},
        bufferDeviceAddressFeatures,
        descriptorIndexingFeatures,
        descriptorBufferFeatures);

    if(supportedDevices.empty())
    {
        fprintf(stdout, "No supported device found\n");
        return EXIT_SUCCESS;
    }

    vkw::Device device;
    check_exit(
        device.init(instance, supportedDevices[0], requiredExtensions, {}, &bufferDeviceAddressFeatures));

    // Storage image descriptor indexing
    fprintf(stdout, "Checking storage image descriptor buffers...\n");
    for(size_t i = 1; i <= 128; ++i)
    {
        const bool res = testStorageImageDescriptorIndexing(device, i, 256);
        fprintf(stdout, "  Descriptor count %zu : %s\n", i, (res == true) ? "OK" : "FAILED");
    }

    return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------------------------------------------------------

bool testStorageImageDescriptorIndexing(
    const vkw::Device& device, const size_t descriptorCount, const size_t imgSize)
{
    std::vector<vkw::StorageImage> imageList{descriptorCount};
    for(auto& image : imageList)
    {
        const VkExtent3D imageExtent{static_cast<uint32_t>(imgSize), static_cast<uint32_t>(imgSize), 1};
        if(!image.init(device, VK_IMAGE_TYPE_2D, VK_FORMAT_R32_SFLOAT, imageExtent))
        {
            fprintf(stderr, "Error initializing image\n");
            return false;
        }
    }

    const VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.pNext = nullptr;
    bindingFlagsCreateInfo.bindingCount = 1;
    bindingFlagsCreateInfo.pBindingFlags = &bindingFlags;

    vkw::DescriptorSetLayout descriptorSetLayout{};
    check_exit(descriptorSetLayout.init(device));
    descriptorSetLayout.addBinding<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_ALL, 0);
    check_exit(descriptorSetLayout.create(
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, &bindingFlagsCreateInfo));

    vkw::DescriptorPool descriptorPool{};
    check_exit(descriptorPool.init(
        device,
        descriptorCount,
        {VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, static_cast<uint32_t>(descriptorCount)}},
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT));

    const uint32_t descriptorArraySize = static_cast<uint32_t>(descriptorCount);
    VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorCountAllocateInfo = {};
    descriptorCountAllocateInfo.sType
        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    descriptorCountAllocateInfo.pNext = nullptr;
    descriptorCountAllocateInfo.descriptorSetCount = 1;
    descriptorCountAllocateInfo.pDescriptorCounts = &descriptorArraySize;

    vkw::DescriptorSet descriptorSet{};
    check_exit(descriptorSet.init(device, descriptorSetLayout, descriptorPool, &descriptorCountAllocateInfo));

    

    vkw::PipelineLayout pipelineLayout{};
    check_exit(pipelineLayout.init(device, descriptorSetLayout));

    struct Params
    {
        uint32_t offset;
        uint32_t range;
    };
    pipelineLayout.reservePushConstants<Params>(vkw::ShaderStage::Compute);
    pipelineLayout.create();

    vkw::ComputePipeline computePipeline{};
    check_exit(computePipeline.init(
        device,
        reinterpret_cast<const char*>(fillStorageImagesDescriptorIndexingComp),
        sizeof(fillStorageImagesDescriptorIndexingComp)));
    check_exit(computePipeline.createPipeline(pipelineLayout));

    vkw::CommandPool cmdPool{device, device.getQueues(vkw::QueueUsageBits::Compute)[0]};
    if(cmdPool.initialized() == false)
    {
        return false;
    }

    auto cmdBuffer = cmdPool.createCommandBuffer();
    cmdBuffer.begin();
    Params params = {0, static_cast<uint32_t>(descriptorCount)};

    cmdBuffer.end();

    vkw::Fence fence(device);

    device.getQueues(vkw::QueueUsageBits::Compute)[0].submit(cmdBuffer, fence);
    fence.wait();

    return true;
}