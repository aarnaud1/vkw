# Copyright (c) 2025 Adrien ARNAUD
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.22)

project(VKW VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SAMPLES "Build samples" OFF)
option(BUILD_FOR_ANDROID "Build for Android" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(VKW_INCLUDE_ROOT include)
set(VKW_SRC_ROOT src)
set(VKW_SRC_FILES
    ${VKW_SRC_ROOT}/BottomLevelAccelerationStructure.cpp
    ${VKW_SRC_ROOT}/ComputePipeline.cpp
    ${VKW_SRC_ROOT}/DebugMessenger.cpp
    ${VKW_SRC_ROOT}/DescriptorPool.cpp
    ${VKW_SRC_ROOT}/DescriptorSet.cpp
    ${VKW_SRC_ROOT}/DescriptorSetLayout.cpp
    ${VKW_SRC_ROOT}/Device.cpp
    ${VKW_SRC_ROOT}/GraphicsPipeline.cpp
    ${VKW_SRC_ROOT}/Instance.cpp
    ${VKW_SRC_ROOT}/PipelineLayout.cpp
    ${VKW_SRC_ROOT}/RenderPass.cpp
    ${VKW_SRC_ROOT}/Surface.cpp
    ${VKW_SRC_ROOT}/Swapchain.cpp
    ${VKW_SRC_ROOT}/Synchronization.cpp
    ${VKW_SRC_ROOT}/TopLevelAccelerationStructure.cpp
    ${VKW_SRC_ROOT}/utils.cpp
)

add_subdirectory(thirdparty/VulkanMemoryAllocator)
add_subdirectory(thirdparty/volk)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG -DERROR_SEVERITY=2)
endif()

## Vulkan
if(BUILD_SAMPLES)
    find_package(Vulkan REQUIRED COMPONENTS glslc)
else()
    find_package(Vulkan REQUIRED)
endif(BUILD_SAMPLES)

## Build library
add_compile_definitions(VK_NO_PROTOTYPES)
add_library(vkw ${VKW_SRC_FILES})
target_include_directories(vkw
    PUBLIC
    ${VKW_INCLUDE_ROOT}
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(vkw
    PUBLIC
    VulkanMemoryAllocator
    Vulkan::Vulkan
    volk_headers
)

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif(BUILD_SAMPLES)
