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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

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
            COMMAND Vulkan::glslc ${arg_GLSLC_OPTIONS} -o ${SPIRV} ${SHADER_SOURCE}
            COMMENT "Running: glslc ${arg_GLSLC_OPTIONS} -o ${SPIRV} ${SHADER_SOURCE}"
            DEPENDS ${SHADER_SOURCE}
        )
        list(APPEND SPIRV_BINARIES ${SPIRV})
    endforeach()
endMacro()

compile_shader_sources(SHADER_SOURCES ${VERTEX_SHADER_FILES} GLSLC_OPTIONS "-std=450core" "-O")
compile_shader_sources(SHADER_SOURCES ${FRAGMENT_SHADER_FILES} GLSLC_OPTIONS "-std=450core" "-O")
compile_shader_sources(SHADER_SOURCES ${COMPUTE_SHADER_FILES} GLSLC_OPTIONS "-std=450core" "-O")
compile_shader_sources(SHADER_SOURCES ${TASK_SHADER_FILES}
                       GLSLC_OPTIONS "-std=450" "--target-env=vulkan1.3" "-O" "-fshader-stage=task")
compile_shader_sources(SHADER_SOURCES ${MESH_SHADER_FILES}
                       GLSLC_OPTIONS "-std=450" "--target-env=vulkan1.3" "-O" "-fshader-stage=mesh")

add_custom_target(Shaders ALL DEPENDS ${SPIRV_BINARIES})

set(SAMPLES_LIST "ArrayAdd"
                 "ArraySaxpy"
                 "MeshShader"
                 "Triangle")
foreach(SAMPLE IN ITEMS ${SAMPLES_LIST})
    add_executable(${SAMPLE} ${SAMPLE}.cpp)
    target_link_libraries(${SAMPLE} vkw glm::glm glfw)
endforeach()