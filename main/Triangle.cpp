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

static constexpr VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;

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
    vk::Instance instance(window);
    vk::Device device(instance);

    // Create buffer
    vk::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto& stagingBuf = stagingMem.createBuffer<Vertex>(hostStagingFlags.usage, vertices.size());
    stagingMem.allocate();

    vk::Memory deviceMem(device, vertexBufferFlags.memoryFlags);
    auto& vertexBuffer = deviceMem.createBuffer<Vertex>(vertexBufferFlags.usage, vertices.size());
    deviceMem.allocate();

    vk::RenderPass renderPass(device);
    renderPass
        .addColorAttachment(
            colorFormat,
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

    vk::PipelineLayout pipelineLayout(device, 0);
    pipelineLayout.create();

    vk::GraphicsPipeline pipeline(device);
    pipeline.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "output/spv/triangle_vert.spv")
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "output/spv/triangle_frag.spv");
    pipeline.addVertexBinding(0, sizeof(Vertex))
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
        .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col));

    pipeline.setViewport(0.0f, 0.0f, float(width), float(height));
    pipeline.setScissors(0, 0, width, height);
    pipeline.setPrimitiveType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline.createPipeline(renderPass, pipelineLayout);

    // Preparing swapchain
    vk::Swapchain swapchain(instance, device, renderPass, width, height, colorFormat);

    // Preparing commands
    vk::CommandPool<vk::QueueFamilyType::GRAPHICS> graphicsCmdPool(device);
    vk::CommandPool<vk::QueueFamilyType::PRESENT> presentCommandPool(device);
    vk::CommandPool<vk::QueueFamilyType::TRANSFER> transferCommandPool(device);
    auto transferCmdBuffer = transferCommandPool.createCommandBuffer();

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
                .bindGraphicsPipeline(pipeline)
                .setViewport(0, 0, float(w), float(h))
                .setScissor({0, 0}, {w, h})
                .bindVertexBuffer(0, vertexBuffer, 0)
                .draw(vertices.size(), 1, 0, 0)
                .endRenderPass()
                .end();
        }
        return graphicsCmdBuffers;
    };
    auto graphicsCmdBuffers = createCommandBuffers(swapchain, width, height);

    vk::Semaphore imageAvailableSemaphore(device);
    vk::Semaphore renderFinishedSemaphore(device);

    // Main loop
    vk::Queue<vk::QueueFamilyType::GRAPHICS> graphicsQueue(device);
    vk::Queue<vk::QueueFamilyType::PRESENT> presentQueue(device);
    vk::Queue<vk::QueueFamilyType::TRANSFER> transferQueue(device);

    stagingMem.copyFromHost<Vertex>(vertices.data(), stagingBuf.getOffset(), vertices.size());
    transferQueue.submit(transferCmdBuffer).waitIdle();

    vk::Fence fence(device, true);
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
            fence = vk::Fence(device, true);
            continue;
        }

        graphicsQueue.submit(
            graphicsCmdBuffers[imageIndex],
            {&imageAvailableSemaphore},
            {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            {&renderFinishedSemaphore},
            fence);
        presentQueue.present(swapchain, {&renderFinishedSemaphore}, imageIndex);
    }

    // Synchronize the queues
    device.waitIdle();

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
