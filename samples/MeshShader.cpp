/*
 * Copyright (C) 2024 Adrien ARNAUD
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

const uint32_t width = 800;
const uint32_t height = 600;

static void runSample(GLFWwindow* window);

static const uint32_t vertexCount = 3;
static const std::vector<glm::vec2> positions = {{-1.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, -1.0f}};
static const std::vector<glm::vec4> colors
    = {{1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}};

static constexpr VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;

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
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<vkw::InstanceExtension> instanceExtensions
        = {vkw::InstanceExtension::SurfaceKhr, vkw::InstanceExtension::XcbSurfaceKhr};
    vkw::Instance instance(instanceLayers, instanceExtensions);
    instance.createSurface(window);

    const std::vector<vkw::DeviceExtension> deviceExtensions
        = {vkw::DeviceExtension::SwapchainKhr, vkw::DeviceExtension::MeshShaderExt};
    const std::vector<VkPhysicalDeviceType> requiredDeviceType
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.pNext = nullptr;
    meshShaderFeatures.taskShader = VK_TRUE;
    meshShaderFeatures.meshShader = VK_TRUE;
    vkw::Device device(instance, deviceExtensions, {}, requiredDeviceType, &meshShaderFeatures);

    auto graphicsQueue = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT)[0];
    auto presentQueue = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_PRESENT_BIT)[0];

    vkw::Memory vertexMemory(
        device,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    auto* vertexBuffer
        = &vertexMemory.createBuffer<glm::vec2>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vertexCount);
    auto* colorBuffer
        = &vertexMemory.createBuffer<glm::vec4>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, vertexCount);
    vertexMemory.allocate();

    vertexMemory.copyFromHost<glm::vec2>(
        positions.data(), vertexBuffer->getMemOffset(), vertexCount);
    vertexMemory.copyFromHost<glm::vec4>(colors.data(), colorBuffer->getMemOffset(), vertexCount);

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
        .create();

    const uint32_t workGroupSize = 3;
    vkw::MeshShaderProgram<> meshProgram(
        device, "output/spv/mesh_shader_mesh.spv", "output/spv/mesh_shader_frag.spv");
    meshProgram.bindStorageBuffer(VK_SHADER_STAGE_MESH_BIT_EXT, 0, *vertexBuffer);
    meshProgram.bindStorageBuffer(VK_SHADER_STAGE_MESH_BIT_EXT, 1, *colorBuffer);
    meshProgram.spec(VK_SHADER_STAGE_MESH_BIT_EXT, workGroupSize);
    meshProgram.setViewport(0.0f, float(height), float(width), -float(height));
    meshProgram.setScissor(0, 0, width, height);
    meshProgram.setCullMode(VK_CULL_MODE_NONE);
    meshProgram.create(renderPass);

    // Preparing swapchain
    vkw::Swapchain swapchain(
        instance,
        device,
        renderPass,
        width,
        height,
        colorFormat,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        false);

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
                    glm::vec4{0.1f, 0.1f, 0.1f, 1.0f})
                .bindMeshShaderProgram(meshProgram)
                .drawMeshTask(1, 1, 1)
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
        auto res = swapchain.getNextImage(imageIndex, imageAvailableSemaphore);
        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            device.waitIdle();

            graphicsCmdBuffers.clear();
            swapchain.reCreate(width, height, VK_FORMAT_B8G8R8A8_SRGB);
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