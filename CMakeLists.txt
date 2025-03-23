# Copyright (C) 2025 Adrien ARNAUD
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.22)

project(VulkanWrappers VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    ${VKW_SRC_ROOT}/TopLevelAccelerationStructure.cpp
    ${VKW_SRC_ROOT}/utils.cpp
)

add_subdirectory(thirdparty/VulkanMemoryAllocator)
add_subdirectory(thirdparty/volk)

option(BUILD_SAMPLES "Build samples" OFF)

## Vulkan
if(BUILD_SAMPLES)
    find_package(Vulkan REQUIRED COMPONENTS glslc)
else()
    find_package(Vulkan REQUIRED)
endif(BUILD_SAMPLES)

## Build library
add_library(vkw ${VKW_SRC_FILES})
add_compile_definitions(VK_NO_PROTOTYPES)
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