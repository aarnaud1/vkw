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

set(VKW_TEST_SRC_FILES
    src/vkw_tests.cpp
    src/testDescriptorIndexing.cpp
)

find_package(Vulkan REQUIRED COMPONENTS glslc)

## List all shader sources
file(GLOB_RECURSE VERTEX_SHADER_FILES "shaders/*.vert")
file(GLOB_RECURSE FRAGMENT_SHADER_FILES "shaders/*.frag")
file(GLOB_RECURSE COMPUTE_SHADER_FILES "shaders/*.comp")
file(GLOB_RECURSE TASK_SHADER_FILES "shaders/*.task")
file(GLOB_RECURSE MESH_SHADER_FILES "shaders/*.mesh")

set(SPIRV_BINARIES)
macro(compile_shader_sources)
    set(glslcArgsList GLSLC_OPTIONS)
    set(shaderSrcList SHADER_SOURCES)
    cmake_parse_arguments(arg "" "" "${glslcArgsList};${shaderSrcList}" ${ARGN})

    foreach(SHADER_SOURCE ${arg_SHADER_SOURCES})
        get_filename_component(FILE_NAME ${SHADER_SOURCE} NAME)
        set(SPIRV "${PROJECT_BINARY_DIR}/spv/${FILE_NAME}.spv")
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/spv/"
            COMMAND Vulkan::glslc -mfmt=num ${arg_GLSLC_OPTIONS} -o ${SPIRV} ${SHADER_SOURCE}
            COMMENT "Running: glslc ${arg_GLSLC_OPTIONS} -o ${SPIRV} ${SHADER_SOURCE}"
            DEPENDS ${SHADER_SOURCE}
        )
        list(APPEND SPIRV_BINARIES ${SPIRV})
    endforeach()
endmacro()

compile_shader_sources(SHADER_SOURCES ${VERTEX_SHADER_FILES}
    GLSLC_OPTIONS "-std=460" "--target-env=vulkan1.3" "--target-spv=spv1.4" "-O")
compile_shader_sources(SHADER_SOURCES ${FRAGMENT_SHADER_FILES}
    GLSLC_OPTIONS "-std=460" "--target-env=vulkan1.3" "--target-spv=spv1.4" "-O")
compile_shader_sources(SHADER_SOURCES ${COMPUTE_SHADER_FILES}
    GLSLC_OPTIONS "-std=460" "--target-env=vulkan1.3" "--target-spv=spv1.4" "-O")
compile_shader_sources(SHADER_SOURCES ${TASK_SHADER_FILES}
    GLSLC_OPTIONS "-std=460" "--target-env=vulkan1.3" "--target-spv=spv1.4" "-O")
compile_shader_sources(SHADER_SOURCES ${MESH_SHADER_FILES}
    GLSLC_OPTIONS "-std=460" "--target-env=vulkan1.3" "--target-spv=spv1.4" "-O")
add_custom_target(Shaders ALL DEPENDS ${SPIRV_BINARIES})


add_executable(vkw-tests ${VKW_TEST_SRC_FILES})
target_link_libraries(vkw-tests vkw)
target_include_directories(vkw-tests PRIVATE include ${PROJECT_BINARY_DIR})
