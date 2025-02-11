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
#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 3

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

static uint32_t currentFrame = 0;
static bool frameResized = false;

const uint32_t initWidth = 800;
const uint32_t initHeight = 600;
VkSurfaceKHR surface = VK_NULL_HANDLE;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void runSample(GLFWwindow* window);

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

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "Triangle", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    runSample(window);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}

void framebufferResizeCallback(GLFWwindow* /*window*/, int /*width*/, int /*height*/)
{
    frameResized = true;
}

void runSample(GLFWwindow* window)
{
    uint32_t glfwExtCount = 0;
    auto* glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    // Init Vulkan
    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> instanceExtensions{};
    for(uint32_t i = 0; i < glfwExtCount; ++i)
    {
        instanceExtensions.push_back(glfwExts[i]);
    }
    vkw::Instance instance(instanceLayers, instanceExtensions);

    glfwCreateWindowSurface(instance.getHandle(), window, nullptr, &surface);
    instance.setSurface(std::move(surface));

    const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
        = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
    const std::vector<const char*> deviceExts = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vkw::Device device(instance, deviceExts, {}, compatibleDeviceTypes);

    auto deviceQueues = device.getQueues(vkw::QueueUsageBits::Graphics);
    if(deviceQueues.empty())
    {
        throw std::runtime_error("No available device queues");
    }
    vkw::Queue graphicsQueue = deviceQueues[0];

    auto presentQueues = device.getQueues(vkw::QueueUsageBits::Present);
    if(presentQueues.empty())
    {
        throw std::runtime_error("No queue available for presntation");
    }
    vkw::Queue presentQueue = presentQueues[0];

    // Create buffer
    vkw::DeviceBuffer<Vertex> vertexBuffer(device, vertexBufferFlags.usage, vertices.size());

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

    vkw::PipelineLayout pipelineLayout(device, 0);
    pipelineLayout.create();

    vkw::GraphicsPipeline graphicsPipeline(device);
    graphicsPipeline.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "build/spv/triangle.vert.spv");
    graphicsPipeline.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "build/spv/triangle.frag.spv");
    graphicsPipeline.addVertexBinding(0, sizeof(Vertex))
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
        .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col));
    graphicsPipeline.createPipeline(renderPass, pipelineLayout);

    // Preparing swapchain
    vkw::Swapchain swapchain(
        instance,
        device,
        renderPass,
        initWidth,
        initHeight,
        MAX_FRAMES_IN_FLIGHT,
        colorFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    // Preparing commands
    vkw::CommandPool graphicsCmdPool(device, graphicsQueue);
    auto recordCommandBuffer
        = [&](auto& cmdBuffer, const uint32_t i, const uint32_t w, const uint32_t h) {
              cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
                  .beginRenderPass(
                      renderPass,
                      swapchain.getFramebuffer(i),
                      VkOffset2D{0, 0},
                      VkExtent2D{w, h},
                      VkClearColorValue{0.1f, 0.1f, 0.1f, 1.0f})
                  .bindGraphicsPipeline(graphicsPipeline)
                  .setViewport(0.0f, 0.0f, float(w), float(h))
                  .setScissor({0, 0}, {w, h})
                  .setCullMode(VK_CULL_MODE_NONE)
                  .bindVertexBuffer(0, vertexBuffer, 0)
                  .draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0)
                  .endRenderPass()
                  .end();
          };
    auto commandBuffers = graphicsCmdPool.createCommandBuffers(MAX_FRAMES_IN_FLIGHT);

    std::vector<vkw::Semaphore> imageAvailableSemaphores;
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        imageAvailableSemaphores[i].init(device);
    }

    std::vector<vkw::Semaphore> renderFinishedSemaphores{};
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        renderFinishedSemaphores[i].init(device);
    }

    // Main loop
    uploadData(device, vertices.data(), vertexBuffer);

    std::vector<vkw::Fence> fences;
    fences.resize(MAX_FRAMES_IN_FLIGHT);
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        fences[i].init(device, true);
    }

    auto recreateSwapchain = [&]() {
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        while(w == 0 || h == 0)
        {
            glfwGetFramebufferSize(window, &w, &h);
            glfwWaitEvents();
        }

        device.waitIdle();
        swapchain.reCreate(w, h);
    };

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        auto& fence = fences[currentFrame];
        fence.wait();

        uint32_t imageIndex;
        auto res = swapchain.getNextImage(
            imageIndex, imageAvailableSemaphores[currentFrame], UINT64_MAX);

        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapchain();
            continue;
        }
        else if(res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Error acquiring the swap chain image");
        }
        fence.reset();

        auto& cmdBuffer = commandBuffers[currentFrame];
        cmdBuffer.reset();
        recordCommandBuffer(
            cmdBuffer, imageIndex, swapchain.getExtent().width, swapchain.getExtent().height);

        res = graphicsQueue.submit(
            cmdBuffer,
            std::vector<vkw::Semaphore*>{&imageAvailableSemaphores[currentFrame]},
            {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            std::vector<vkw::Semaphore*>{&renderFinishedSemaphores[currentFrame]},
            fence);
        if(res != VK_SUCCESS)
        {
            throw std::runtime_error("Error submitting graphics commands");
        }

        res = presentQueue.present(
            swapchain,
            std::vector<vkw::Semaphore*>{&renderFinishedSemaphores[currentFrame]},
            imageIndex);
        if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || frameResized)
        {
            recreateSwapchain();
            frameResized = false;
        }
        else if(res != VK_SUCCESS)
        {
            throw std::runtime_error("Error presenting image");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    // Synchronize the queues
    device.waitIdle();
}