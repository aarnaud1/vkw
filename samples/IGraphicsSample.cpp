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

#include "IGraphicsSample.hpp"

#include <cstdio>
#include <cstdlib>

IGraphicsSample::IGraphicsSample(
    const uint32_t frameWidth,
    const uint32_t frameHeight,
    const std::vector<const char*>& instanceExtensions)
    : frameWidth_{frameWidth}, frameHeight_{frameHeight}, instanceExtensions_{instanceExtensions}
{
    instanceLayers_.push_back("VK_LAYER_KHRONOS_validation");

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
}

bool IGraphicsSample::initSample()
{
    VKW_CHECK_BOOL_RETURN_FALSE(instance_.init(instanceLayers_, instanceExtensions_));

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
    if(graphicsQueues.empty())
    {
        fprintf(stderr, "Not graphics queue found\n");
        return false;
    }
    graphicsQueue_ = graphicsQueues[0];

    cmdPool_.init(device_, graphicsQueue_);
    initCmdBuffers_ = cmdPool_.createCommandBuffers(framesInFlight);
    drawCmdBuffers_ = cmdPool_.createCommandBuffers(framesInFlight);
    postDrawCmdBuffers_ = cmdPool_.createCommandBuffers(framesInFlight);

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

    return true;
}

bool IGraphicsSample::setSurface(VkSurfaceKHR&& surface)
{
    VKW_CHECK_BOOL_RETURN_FALSE(surface_.init(instance_, std::move(surface)));

    const auto presentQueues = device_.getPresentQueues(surface_);
    if(presentQueues.empty())
    {
        fprintf(stderr, "Present not supported\n");
        return false;
    }
    presentQueue_ = presentQueues[0];

    VKW_CHECK_BOOL_RETURN_FALSE(swapchain_.init(
        surface_,
        device_,
        frameWidth_,
        frameHeight_,
        framesInFlight,
        colorFormat,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        colorSpace));

    initImageLayouts();

    return true;
}

bool IGraphicsSample::render()
{
    static uint32_t frameIndex = 0;
    uint32_t imageIndex;

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
        return false;
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
        needsResize_ = false;
    }
    else if(res != VK_SUCCESS)
    {
        throw std::runtime_error("Error presenting image");
    }

    // Perform post draw operations
    const bool postDrawRecorded = recordPostDrawCommands(postDrawCmdBuffer, frameIndex, imageIndex);
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

    return true;
}

void IGraphicsSample::finalize() { device_.waitIdle(); }

void IGraphicsSample::resize(const uint32_t w, const uint32_t h)
{
    device_.waitIdle();
    swapchain_.reCreate(w, h);

    frameWidth_ = w;
    frameHeight_ = h;
    initImageLayouts();
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