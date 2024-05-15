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

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>

#include <glm/glm.hpp>

#include "Common.hpp"

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 col;
};
const std::vector<Vertex> vertices
    = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
       {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
       {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Triangle", nullptr, nullptr);

    // Init Vulkan
    vk::Instance instance(window);
    vk::Device device(instance);
    VkSwapchainKHR swapChain;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = instance.getSurface();
    createInfo.minImageCount = 2;
    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = {800, 600};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    CHECK_VK(
        vkCreateSwapchainKHR(device.getHandle(), &createInfo, nullptr, &swapChain),
        "Creating swapchain");

    uint32_t imageCount;
    std::vector<VkImage> swapChainImages;

    vkGetSwapchainImagesKHR(device.getHandle(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.getHandle(), swapChain, &imageCount, swapChainImages.data());

    std::vector<VkImageView> swapChainImageViews;
    swapChainImageViews.resize(swapChainImages.size());

    for(size_t i = 0; i < swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        CHECK_VK(
            vkCreateImageView(device.getHandle(), &createInfo, nullptr, &swapChainImageViews[i]),
            "Creating image view");
    }

    // Create buffer
    vk::Memory stagingMem(device, hostStagingFlags.memoryFlags);
    auto& stagingBuf = stagingMem.createBuffer<Vertex>(hostStagingFlags.usage, vertices.size());
    stagingMem.allocate();

    vk::Memory deviceMem(device, vertexBufferFlags.memoryFlags);
    auto& vertexBuffer = deviceMem.createBuffer<Vertex>(vertexBufferFlags.usage, vertices.size());
    deviceMem.allocate();

    VkAttachmentDescription attachment{};
    attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vk::RenderPass renderPass(device);
    renderPass.addAttachment(attachment);
    renderPass.addSubPass(subpass);
    renderPass.addSubpassDependency(dependency);
    renderPass.create();

    vk::PipelineLayout pipelineLayout(device, 0);
    pipelineLayout.create();

    vk::GraphicsPipeline pipeline(device);
    pipeline.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "output/spv/triangle_vert.spv")
        .addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "output/spv/triangle_frag.spv");
    pipeline.addVertexBinding(0, sizeof(Vertex))
        .addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
        .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col));

    pipeline.setViewport(0.0f, 0.0f, 800.0f, 600.0f);
    pipeline.setScissors(0, 0, 800, 600);
    pipeline.setPrimitiveType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline.createPipeline(renderPass, pipelineLayout);

    // Get framebuffer
    std::vector<VkFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {swapChainImageViews[i]};
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass.getHandle();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = 800;
        framebufferInfo.height = 600;
        framebufferInfo.layers = 1;

        CHECK_VK(
            vkCreateFramebuffer(
                device.getHandle(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]),
            "Creating framebuffer");
    }

    // Preparing commands
    vk::CommandPool<vk::QueueFamilyType::GRAPHICS> graphicsCmdPool(device);
    vk::CommandPool<vk::QueueFamilyType::PRESENT> presentCommandPool(device);
    vk::CommandPool<vk::QueueFamilyType::TRANSFER> transferCommandPool(device);
    auto transferCmdBuffer = transferCommandPool.createCommandBuffer();

    std::array<VkBufferCopy, 1> c0 = {{0, 0, vertices.size() * sizeof(Vertex)}};
    transferCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
        .copyBuffer(stagingBuf, vertexBuffer, c0)
        .end();

    auto graphicsCmdBuffers = graphicsCmdPool.createCommandBuffers(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainImageViews.size(); ++i)
    {
        auto& graphicsCmdBuffer = graphicsCmdBuffers[i];
        graphicsCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
            .beginRenderPass(
                renderPass,
                swapChainFramebuffers[i],
                VkOffset2D{0, 0},
                VkExtent2D{800, 600},
                glm::vec4{0.1f, 0.1f, 0.1f, 1.0f})
            .bindGraphicsPipeline(pipeline)
            .bindVertexBuffer(0, vertexBuffer, 0)
            .draw(vertices.size(), 1, 0, 0)
            .endRenderPass()
            .end();
    }

    vk::Semaphore imageAvailableSemaphore(device);
    vk::Semaphore renderFinishedSemaphore(device);

    // Main loop
    vk::Queue<vk::QueueFamilyType::GRAPHICS> graphicsQueue(device);
    vk::Queue<vk::QueueFamilyType::PRESENT> presentQueue(device);
    vk::Queue<vk::QueueFamilyType::TRANSFER> transferQueue(device);

    stagingMem.copyFromHost<Vertex>(vertices.data(), stagingBuf.getOffset(), vertices.size());
    transferQueue.submit(transferCmdBuffer).waitIdle();
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Draw frame
        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            device.getHandle(),
            swapChain,
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphore.getHandle(),
            VK_NULL_HANDLE,
            &imageIndex);

        graphicsQueue
            .submit(
                graphicsCmdBuffers[imageIndex],
                {&imageAvailableSemaphore},
                {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                {&renderFinishedSemaphore})
            .waitIdle();

        presentQueue.present(swapChain, {&renderFinishedSemaphore}, imageIndex).waitIdle();
    }

    for(auto framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device.getHandle(), framebuffer, nullptr);
    }

    for(auto imageView : swapChainImageViews)
    {
        vkDestroyImageView(device.getHandle(), imageView, nullptr);
    }
    vkDestroySwapchainKHR(device.getHandle(), swapChain, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}