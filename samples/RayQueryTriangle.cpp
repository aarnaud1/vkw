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

#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define MAX_FRAMES_IN_FLIGHT 3
const uint32_t initWidth = 800;
const uint32_t initHeight = 600;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void runSample(GLFWwindow* window);

static uint32_t currentFrame = 0;
static bool frameResized = false;

const std::vector<glm::vec3> vertices
    = {{0.0f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}};
const std::vector<uint32_t> indices = {0, 1, 2};

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

void runSample(GLFWwindow* window)
{
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

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

    glfwCreateWindowSurface(instance.getHandle(), window, nullptr, &vkSurface);
    vkw::Surface surface(instance, std::move(vkSurface));

    const std::vector<const char*> deviceExts
        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
           VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
           VK_KHR_RAY_QUERY_EXTENSION_NAME,
           VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME};

    VkPhysicalDeviceAccelerationStructureFeaturesKHR deviceAccelerationStructureFeature = {};
    deviceAccelerationStructureFeature.sType
        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    deviceAccelerationStructureFeature.pNext = nullptr;
    deviceAccelerationStructureFeature.accelerationStructure = VK_TRUE;
    deviceAccelerationStructureFeature.accelerationStructureCaptureReplay = VK_FALSE;
    deviceAccelerationStructureFeature.accelerationStructureHostCommands = VK_FALSE;
    deviceAccelerationStructureFeature.accelerationStructureIndirectBuild = VK_FALSE;
    deviceAccelerationStructureFeature.descriptorBindingAccelerationStructureUpdateAfterBind
        = VK_FALSE;

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddressFeature = {};
    deviceAddressFeature.sType
        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
    deviceAddressFeature.pNext = &deviceAccelerationStructureFeature;
    deviceAddressFeature.bufferDeviceAddressCaptureReplay = VK_FALSE;
    deviceAddressFeature.bufferDeviceAddressMultiDevice = VK_FALSE;
    deviceAddressFeature.bufferDeviceAddress = VK_TRUE;

    const VkPhysicalDevice physicalDevice = findCompatibleDevice(
        instance, deviceExts, deviceAccelerationStructureFeature, deviceAddressFeature);
    vkw::Device device(instance, physicalDevice, deviceExts, {}, &deviceAddressFeature);

    // Build acceleration structure
    const auto transform = vkw::asIdentityMatrix;
    vkw::TriangleDataVec3i32<> geometryData{
        vertices.data(), indices.data(), &transform, 3, sizeof(glm::vec4), 1};
    vkw::BottomLevelAccelerationStructure blas{device, true};
    blas.addGeometry(geometryData).create();
    fprintf(stdout, "BLAS created\n");
}

void framebufferResizeCallback(GLFWwindow* /*window*/, int /*width*/, int /*height*/)
{
    frameResized = true;
}
