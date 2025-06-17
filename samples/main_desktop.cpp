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
#include "SimpleTriangle.hpp"

#include <cstdlib>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static constexpr uint32_t initWidth = 800;
static constexpr uint32_t initHeight = 600;
static std::unique_ptr<IGraphicsSample> graphicsSample = nullptr;

enum class SampleType : uint32_t
{
    SimpleTriangle = 0,
    RayQueryTriangle = 1,
    TestCaseCount = 2
};

int main(int argc, char** argv)
{
    if(argc <= 1)
    {
        fprintf(stderr, "Not enough arguments");
        return EXIT_FAILURE;
    }

    const uint32_t testCase = atoi(argv[1]);
    if(testCase >= static_cast<uint32_t>(SampleType::TestCaseCount))
    {
        fprintf(stderr, "Ibvalid test case");
        return EXIT_FAILURE;
    }

    VKW_CHECK_BOOL_FAIL(glfwInit(), "Error initializing GLFW");
    VKW_CHECK_BOOL_FAIL(glfwVulkanSupported(), "Error: Vulkan nor supported on this device");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "VKW sample", nullptr, nullptr);

    uint32_t instanceExtensionCount = 0;
    const auto* instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionCount);
    std::vector<const char*> requiredInstanceExtensions = {};
    for(uint32_t i = 0; i < instanceExtensionCount; ++i)
    {
        requiredInstanceExtensions.push_back(instanceExtensions[i]);
    }

    try
    {
        static const uint32_t fboWidth = 800;
        static const uint32_t fboHeight = 600;
        switch(static_cast<SampleType>(testCase))
        {
            case SampleType::SimpleTriangle:
                graphicsSample.reset(
                    new SimpleTriangle(fboWidth, fboHeight, requiredInstanceExtensions));
                break;
            case SampleType::RayQueryTriangle:
                graphicsSample.reset(
                    new RayQueryTriangle(fboWidth, fboHeight, requiredInstanceExtensions));
                break;
            default:
                break;
        }

        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if(graphicsSample != nullptr)
        {
            glfwSetWindowUserPointer(window, graphicsSample.get());
            VKW_CHECK_BOOL_FAIL(graphicsSample->initSample(), "Error initializing sample");
            VKW_CHECK_VK_FAIL(
                glfwCreateWindowSurface(
                    graphicsSample->instance().getHandle(), window, nullptr, &surface),
                "Error creating surface");
            VKW_CHECK_BOOL_FAIL(
                graphicsSample->setSurface(std::move(surface)), "Error initializing surface");

            while(!glfwWindowShouldClose(window))
            {
                if(graphicsSample->render() == false)
                {
                    int w, h;
                    glfwGetFramebufferSize(window, &w, &h);
                    while((w == 0) | (h == 0))
                    {
                        glfwGetFramebufferSize(window, &w, &h);
                        glfwWaitEvents();
                    }
                    graphicsSample->resize(w, h);
                }
                glfwPollEvents();
            }
            graphicsSample->finalize();
        }
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return EXIT_FAILURE;
    }

    graphicsSample.reset();

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}