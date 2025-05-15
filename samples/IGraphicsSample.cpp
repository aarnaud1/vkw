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

#include "IGraphicsSample.hpp"

#include <cstdio>
#include <cstdlib>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

IGraphicsSample::IGraphicsSample()
{
    VKW_CHECK_BOOL_FAIL(glfwInit(), "Error initializing GLFW");
    VKW_CHECK_BOOL_FAIL(glfwVulkanSupported(), "Error: Vulkan nor supported on this device");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(initWidth, initHeight, "Triangle", nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);

    instanceLayers_.push_back("VK_LAYER_KHRONOS_validation");

    uint32_t instanceExtensionCount = 0;
    const auto* instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    std::vector<const char*> requiredInstanceExtensions = {};
    for(uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        instanceExtensions_.push_back(instanceExtensions[i]);
    }

    // This will be completed in the constructor for the subclasses
    deviceFeatures_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures_.pNext = nullptr;
    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

IGraphicsSample::~IGraphicsSample()
{
    postDrawCmdBuffers_.clear();
    drawCmdBuffers_.clear();
    initCmdBuffers_.clear();
    cmdPool_.clear();

    renderSemaphores_.clear();
    imgSemaphores_.clear();
    frameFences_.clear();
    swapchain_.clear();

    device_.clear();
    surface_.clear();
    instance_.clear();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool IGraphicsSample::initSample()
{
    VKW_CHECK_BOOL_RETURN_FALSE(instance_.init(instanceLayers_, instanceExtensions_));

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VKW_CHECK_VK_RETURN_FALSE(
        glfwCreateWindowSurface(instance_.getHandle(), window_, nullptr, &surface));
    VKW_CHECK_BOOL_RETURN_FALSE(surface_.init(instance_, std::move(surface)));

    const auto physicalDevice = findSupportedDevice();
    if(physicalDevice == VK_NULL_HANDLE)
    {
        fprintf(stderr, "Error: no supported device for this sample\n");
        return false;
    }
    VKW_CHECK_BOOL_RETURN_FALSE(device_.init(
        instance_,
        physicalDevice,
        deviceExtensions_,
        deviceFeatures_.features,
        deviceFeatures_.pNext));
    const auto graphicsQueues
        = device_.getQueues(vkw::QueueUsageBits::Graphics | vkw::QueueUsageBits::Compute);
    const auto presentQueues = device_.getPresentQueues(surface_);
    if(graphicsQueues.empty() || presentQueues.empty())
    {
        fprintf(stderr, "Not all operations supported for this sample\n");
        return false;
    }
    graphicsQueue_ = graphicsQueues[0];
    presentQueue_ = presentQueues[0];

    cmdPool_.init(device_, graphicsQueue_);
    initCmdBuffers_ = cmdPool_.createCommandBuffers(framesInFlight);
    drawCmdBuffers_ = cmdPool_.createCommandBuffers(framesInFlight);
    postDrawCmdBuffers_ = cmdPool_.createCommandBuffers(framesInFlight);

    VKW_CHECK_BOOL_RETURN_FALSE(swapchain_.init(
        surface_,
        device_,
        initWidth,
        initHeight,
        framesInFlight,
        colorFormat,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        colorSpace));

    frameFences_.resize(framesInFlight);
    imgSemaphores_.resize(framesInFlight);
    renderSemaphores_.resize(framesInFlight);
    for(uint32_t i = 0; i < framesInFlight; ++i)
    {
        VKW_CHECK_BOOL_RETURN_FALSE(frameFences_[i].init(device_, true));
        VKW_CHECK_BOOL_RETURN_FALSE(imgSemaphores_[i].init(device_));
        VKW_CHECK_BOOL_RETURN_FALSE(renderSemaphores_[i].init(device_));
    }
    VKW_CHECK_BOOL_RETURN_FALSE(this->init());

    initImageLayouts();
    return true;
}

bool IGraphicsSample::runSample()
{
    std::vector<vkw::Fence> initFences{};
    for(uint32_t id = 0; id < framesInFlight; ++id)
    {
        const bool initRecorded = recordInitCommands(initCmdBuffers_[id], id);
        if(initRecorded)
        {
            vkw::Fence initFence{device_, false};
            graphicsQueue_.submit(initCmdBuffers_[id], initFence);
            initFences.emplace_back(std::move(initFence));
        }
    }
    vkw::Fence::wait(device_, initFences);
    initFences.clear();

    uint32_t imageIndex;
    uint32_t frameIndex = 0;
    while(!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();

        auto& fence = frameFences_[frameIndex];
        auto& imgSemaphore = imgSemaphores_[frameIndex];
        auto& renderSemaphore = renderSemaphores_[frameIndex];

        auto& drawCmdBuffer = drawCmdBuffers_[frameIndex];
        auto& postDrawCmdBuffer = postDrawCmdBuffers_[frameIndex];

        fence.wait();

        VkResult res = VK_SUCCESS;
        res = swapchain_.getNextImage(imageIndex, imgSemaphore, UINT64_MAX);

        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            handleResize();
            continue;
        }
        else if((res != VK_SUCCESS) && (res != VK_SUBOPTIMAL_KHR))
        {
            throw std::runtime_error("Error acquiring the swap chain image");
        }
        fence.reset();

        // Perform draw
        recordDrawCommands(drawCmdBuffer, frameIndex, imageIndex);
        res = graphicsQueue_.submit(
            drawCmdBuffer,
            std::vector<vkw::Semaphore*>{&imgSemaphore},
            std::vector<VkPipelineStageFlags>{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            std::vector<vkw::Semaphore*>{&renderSemaphore},
            fence);
        if(res != VK_SUCCESS)
        {
            throw std::runtime_error("Error submitting graphics commands");
        }

        res = presentQueue_.present(
            swapchain_, std::vector<vkw::Semaphore*>{&renderSemaphore}, imageIndex);
        if((res == VK_ERROR_OUT_OF_DATE_KHR) || (res == VK_SUBOPTIMAL_KHR) || needsResize_)
        {
            handleResize();
            needsResize_ = false;
        }
        else if(res != VK_SUCCESS)
        {
            throw std::runtime_error("Error presenting image");
        }

        // Perform post draw operations
        const bool postDrawRecorded
            = recordPostDrawCommands(postDrawCmdBuffer, frameIndex, imageIndex);
        if(postDrawRecorded)
        {
            auto postDrawFence = vkw::Fence{device_, false};
            graphicsQueue_.submit(
                postDrawCmdBuffer,
                std::vector<vkw::Semaphore*>{&renderSemaphore},
                std::vector<VkPipelineStageFlags>{VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT},
                std::vector<vkw::Semaphore*>{},
                postDrawFence);
            postDrawFence.wait();
            postDraw();
        }

        frameIndex = (frameIndex + 1) % framesInFlight;
        device_.waitIdle(); // Remove after debug
    }
    device_.waitIdle();

    return true;
}

void IGraphicsSample::handleResize()
{
    int w, h;
    glfwGetFramebufferSize(window_, &w, &h);
    while(w == 0 || h == 0)
    {
        glfwGetFramebufferSize(window_, &w, &h);
        glfwWaitEvents();
    }

    device_.waitIdle();
    swapchain_.reCreate(w, h);

    frameWidth_ = w;
    frameHeight_ = h;
    initImageLayouts();
}

void framebufferResizeCallback(GLFWwindow* window, int /*width*/, int /*height*/)
{
    auto* samplePtr = reinterpret_cast<IGraphicsSample*>(glfwGetWindowUserPointer(window));
    samplePtr->requestResize();
}

void IGraphicsSample::initImageLayouts()
{
    auto cmdBuffer = cmdPool_.createCommandBuffer();
    cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // Initialize swapchain images
    std::vector<VkImageMemoryBarrier> memoryBarriers{};
    for(uint32_t i = 0; i < swapchain_.imageCount(); ++i)
    {
        VkImageMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.image = swapchain_.images()[i];
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        memoryBarriers.push_back(memoryBarrier);
    }
    cmdBuffer.imageMemoryBarriers(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, memoryBarriers);
    cmdBuffer.end();

    auto initFence = vkw::Fence(device_, false);
    graphicsQueue_.submit(cmdBuffer, initFence);
    initFence.wait();
}