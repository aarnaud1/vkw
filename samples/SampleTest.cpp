#include "vkWrappers/wrappers.hpp"

#include <cstdlib>

#ifndef GLFW_INCLUDE_VULKAN
#    define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

int main(int, char**)
{
    if(!glfwInit())
    {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(512, 512, "Main window", nullptr, nullptr);

    const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<vkw::InstanceExtension> instanceExts
        = {vkw::DebugUtilsExt, vkw::SurfaceKhr, vkw::XcbSurfaceKhr};
    vkw::Instance instance{instanceLayers, instanceExts};

    if(!instance.createSurface(window))
    {
        fprintf(stderr, "Error creating window\n");
        return EXIT_FAILURE;
    }

    vkw::DebugMessenger messenger(instance);

    return EXIT_SUCCESS;
}