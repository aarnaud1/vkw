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
#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 col;
};
const std::vector<Vertex> vertices
    = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
       {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
       {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

static constexpr VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

int main(int, char**)
{
    const uint32_t width = 800;
    const uint32_t height = 600;

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

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Triangle", nullptr, nullptr);

    // Init Vulkan
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<vkw::InstanceExtension> instanceExts
        = {vkw::DebugUtilsExt, vkw::SurfaceKhr, vkw::XcbSurfaceKhr};
    vkw::Instance instance(instanceLayers, instanceExts);
    instance.createSurface(window);

    const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    const std::vector<vkw::DeviceExtension> deviceExts = {vkw::SwapchainKhr};
    vkw::Device device(instance, deviceExts, {}, compatibleDeviceTypes);

    auto deviceQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT);
    if(deviceQueues.empty())
    {
        throw std::runtime_error("No available device queues");
    }
    vkw::Queue graphicsQueue = deviceQueues[0];

    auto presentQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_PRESENT_BIT);
    if(presentQueues.empty())
    {
        throw std::runtime_error("No queue available for presntation");
    }
    vkw::Queue presentQueue = presentQueues[0];

    // Create buffer
    vkw::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto stagingBuf = stagingMem.createBuffer<Vertex>(hostStagingFlags.usage, vertices.size());
    stagingMem.allocate();

    vkw::Memory deviceMem(device, vertexBufferFlags.memoryFlags);
    auto vertexBuffer = deviceMem.createBuffer<Vertex>(vertexBufferFlags.usage, vertices.size());
    deviceMem.allocate();

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

    struct GraphicsProgramConstants
    {};
    vkw::GraphicsProgram<GraphicsProgramConstants> graphicsProgram(
        device, "output/spv/triangle_vert.spv", "output/spv/triangle_frag.spv");
    graphicsProgram.bindVertexBuffer(vertexBuffer)
        .vertexAttribute(0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
        .vertexAttribute(1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col));
    graphicsProgram.setViewport(0.0f, 0.0f, float(width), float(height));
    graphicsProgram.setScissor(0, 0, width, height);
    graphicsProgram.create(renderPass);

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

    std::array<VkBufferCopy, 1> c0 = {{0, 0, vertices.size() * sizeof(Vertex)}};
    transferCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(stagingBuf, vertexBuffer, c0)
        .end();

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
                .bindGraphicsProgram(graphicsProgram)
                .bindVertexBuffer(0, vertexBuffer, 0)
                .draw(vertices.size(), 1, 0, 0)
                .endRenderPass()
                .end();
        }
        return graphicsCmdBuffers;
    };
    auto graphicsCmdBuffers = createCommandBuffers(swapchain, width, height);

    vkw::Semaphore imageAvailableSemaphore(device);
    vkw::Semaphore renderFinishedSemaphore(device);

    // Main loop
    stagingMem.copyFromHost<Vertex>(vertices.data(), stagingBuf.getMemOffset(), vertices.size());
    graphicsQueue.submit(transferCmdBuffer).waitIdle();

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
            swapchain.reCreate(width, height, colorFormat);
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

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
