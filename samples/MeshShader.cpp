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

#include "Common.hpp"

#include <cstdio>
#include <cstdlib>
#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t width = 800;
const uint32_t height = 600;

static void runSample(GLFWwindow* window);

static const uint32_t vertexCount = 3;
static const std::vector<glm::vec2> positions = {{-1.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, -1.0f}};
static const std::vector<glm::vec4> colors
    = {{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}};

static constexpr VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

int main(int, char**)
{
    if(!glfwInit())
    {
        fprintf(stderr, "Error initializing GLFW\n");
        return EXIT_FAILURE;
    }

    if(!glfwVulkanSupported())
    {
        fprintf(stderr, "Vulkan not supported\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Triangle", nullptr, nullptr);

    try
    {
        runSample(window);
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}

void runSample(GLFWwindow* window)
{
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

    uint32_t glfwExtCount = 0;
    auto* glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> instanceExtensions{};
    for(uint32_t i = 0; i < glfwExtCount; ++i)
    {
        instanceExtensions.push_back(glfwExts[i]);
    }

    vkw::Instance instance(instanceLayers, instanceExtensions);
    glfwCreateWindowSurface(instance.getHandle(), window, nullptr, &vkSurface);

    vkw::Surface surface(instance, std::move(vkSurface));

    const std::vector<const char*> deviceExtensions
        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MESH_SHADER_EXTENSION_NAME};
    const std::vector<VkPhysicalDeviceType> requiredDeviceType
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.pNext = nullptr;
    meshShaderFeatures.taskShader = VK_TRUE;
    meshShaderFeatures.meshShader = VK_TRUE;

    VkPhysicalDeviceMaintenance4Features maintenance4Features{};
    maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
    maintenance4Features.pNext = &meshShaderFeatures;
    maintenance4Features.maintenance4 = VK_TRUE;
    vkw::Device device(instance, deviceExtensions, {}, requiredDeviceType, &maintenance4Features);

    auto graphicsQueue = device.getQueues(vkw::QueueUsageBits::Graphics)[0];
    auto presentQueue = device.getPresentQueues(surface)[0];

    vkw::DeviceBuffer<glm::vec2> vertexBuffer(
        device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vertexCount);
    vkw::DeviceBuffer<glm::vec4> colorBuffer(
        device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vertexCount);

    uploadData(device, positions.data(), vertexBuffer);
    uploadData(device, colors.data(), colorBuffer);

    vkw::RenderPass renderPass(device);
    renderPass
        .addColorAttachment(
            colorFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_SAMPLE_COUNT_1_BIT)
        .addSubPass({0})
        .addSubpassDependency(
            VK_SUBPASS_EXTERNAL,
            0,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
        .create();

    vkw::PipelineLayout pipelineLayout(device, 1);
    pipelineLayout.getDescriptorSetLayout(0)
        .addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_MESH_BIT_EXT, 0)
        .addBinding<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_MESH_BIT_EXT, 1);
    pipelineLayout.create();

    const uint32_t workGroupSize = 3;
    vkw::GraphicsPipeline meshGraphicsPipeline(device);
    meshGraphicsPipeline.addShaderStage(
        VK_SHADER_STAGE_MESH_BIT_EXT, "build/spv/mesh_shader.mesh.spv");
    meshGraphicsPipeline.addShaderStage(
        VK_SHADER_STAGE_FRAGMENT_BIT, "build/spv/mesh_shader.frag.spv");
    meshGraphicsPipeline.addSpec<uint32_t>(VK_SHADER_STAGE_MESH_BIT_EXT, workGroupSize);
    meshGraphicsPipeline.createPipeline(renderPass, pipelineLayout);

    // Allocate descriptor sets
    vkw::DescriptorPool descriptorPool(device, 1, 16);
    auto descriptorSet
        = descriptorPool.allocateDescriptorSet(pipelineLayout.getDescriptorSetLayout(0));
    descriptorSet.bindStorageBuffer(0, vertexBuffer);
    descriptorSet.bindStorageBuffer(1, colorBuffer);

    // Preparing swapchain
    vkw::Swapchain swapchain(
        surface,
        device,
        renderPass,
        width,
        height,
        3,
        colorFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    // Preparing commands
    vkw::CommandPool graphicsCmdPool(device, graphicsQueue);
    auto transferCmdBuffer = graphicsCmdPool.createCommandBuffer();

    auto createCommandBuffers = [&](auto& swapchain, const uint32_t w, const uint32_t h) {
        auto graphicsCmdBuffers = graphicsCmdPool.createCommandBuffers(swapchain.imageCount());
        for(size_t i = 0; i < swapchain.imageCount(); ++i)
        {
            auto& graphicsCmdBuffer = graphicsCmdBuffers[i];
            graphicsCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
                .beginRenderPass(
                    renderPass,
                    swapchain.getFramebuffer(i),
                    VkOffset2D{0, 0},
                    VkExtent2D{w, h},
                    VkClearColorValue{0.1f, 0.1f, 0.1f, 1.0f})
                .bindGraphicsPipeline(meshGraphicsPipeline)
                .bindGraphicsDescriptorSet(pipelineLayout, 0, descriptorSet)
                .setViewport(0.0f, float(height), float(width), -float(height))
                .setScissor({0, 0}, {width, height})
                .setCullMode(VK_CULL_MODE_NONE)
                .drawMeshTasks(1, 1, 1)
                .endRenderPass()
                .end();
        }
        return graphicsCmdBuffers;
    };
    auto graphicsCmdBuffers = createCommandBuffers(swapchain, width, height);

    vkw::Semaphore imageAvailableSemaphore(device);
    vkw::Semaphore renderFinishedSemaphore(device);

    // Main loop
    vkw::Fence fence(device, true);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Draw frame
        fence.waitAndReset();

        uint32_t imageIndex;
        auto res = swapchain.getNextImage(imageIndex, imageAvailableSemaphore, 1000);
        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            device.waitIdle();

            graphicsCmdBuffers.clear();
            swapchain.reCreate(width, height);

            graphicsCmdBuffers = createCommandBuffers(
                swapchain, swapchain.getExtent().width, swapchain.getExtent().height);
            fence = vkw::Fence(device, true);
            continue;
        }

        graphicsQueue.submit(
            graphicsCmdBuffers[imageIndex],
            std::vector<vkw::Semaphore*>{&imageAvailableSemaphore},
            {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            std::vector<vkw::Semaphore*>{&renderFinishedSemaphore},
            fence);
        presentQueue.present(
            swapchain, std::vector<vkw::Semaphore*>{&renderFinishedSemaphore}, imageIndex);
    }

    // Synchronize the queues
    device.waitIdle();
}