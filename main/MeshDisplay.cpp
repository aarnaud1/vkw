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

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include <tinyply.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Common.hpp"

namespace ply = tinyply;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
};

// Utility functions
namespace utils
{
static std::tuple<glm::vec3, glm::vec3, glm::vec3> readPLY(
    const char* filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indieces);
}

class Engine
{
  public:
    Engine(GLFWwindow* window, const uint32_t width, const uint32_t height)
        : window_{window}
        , instance_{window_}
        , device_{instance_}
        , stagingMem_{device_, hostStagingFlags.memoryFlags}
        , deviceMem_{device_, deviceFlags.memoryFlags}
        , width_{width}
        , height_(height)
    {}

    ~Engine() { device_.waitIdle(); }

    void readPLY(const char* filename)
    {
        const auto [centroid, bboxMin, bboxMax] = utils::readPLY(filename, vertices_, indices_);
        centroid_ = centroid;
        bboxMin_ = bboxMin;
        bboxMax_ = bboxMax;
    }

    void allocateBuffers()
    {
        const size_t maxStagingSize
            = std::max(vertices_.size() * sizeof(Vertex), indices_.size() * sizeof(uint32_t));
        stagingBuffer_ = &stagingMem_.createBuffer<uint8_t>(hostStagingFlags.usage, maxStagingSize);

        vertexBuffer_ = &deviceMem_.createBuffer<Vertex>(vertexBufferFlags.usage, vertices_.size());
        indexBuffer_ = &deviceMem_.createBuffer<uint32_t>(indexBufferFlags.usage, indices_.size());

        // Allocate memory
        stagingMem_.allocate();
        deviceMem_.allocate();
    }

    void allocatePipeline()
    {
        // Graphics pipeline creation
        graphicsPipelineLayout_.reset(new vk::PipelineLayout(device_, 1));
        graphicsPipelineLayout_->getDescriptorSetlayoutInfo(0).addUniformBufferBinding(
            VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
        graphicsPipelineLayout_->create();

        renderPass_.reset(new vk::RenderPass(device_));
        renderPass_->addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB, VK_SAMPLE_COUNT_1_BIT)
            .addSubPass({0})
            .addSubpassDependency(
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
            .create();

        graphicsPipeline_.reset(new vk::GraphicsPipeline(device_));
        graphicsPipeline_->addShaderStage(
            VK_SHADER_STAGE_VERTEX_BIT, "output/spv/mesh_display_vert.spv");
        graphicsPipeline_->addShaderStage(
            VK_SHADER_STAGE_FRAGMENT_BIT, "output/spv/mesh_display_frag.spv");
        graphicsPipeline_->addVertexBinding(0, sizeof(Vertex));
        graphicsPipeline_->addVertexAttribute(
            0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
        graphicsPipeline_->addVertexAttribute(
            1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
        graphicsPipeline_->addVertexAttribute(
            2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal));

        graphicsPipeline_->setViewport(0.0f, 0.0f, float(width_), float(height_));
        graphicsPipeline_->setScissors(0, 0, width_, height_);
        graphicsPipeline_->setPrimitiveType(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        graphicsPipeline_->createPipeline(*renderPass_, *graphicsPipelineLayout_);

        graphicsCmdPool_.reset(new vk::CommandPool<vk::QueueFamilyType::GRAPHICS>(device_));
        transferCmdPool_.reset(new vk::CommandPool<vk::QueueFamilyType::TRANSFER>(device_));

        swapchain_.reset(new vk::Swapchain(
            instance_, device_, *renderPass_, width_, height_, VK_FORMAT_B8G8R8A8_SRGB));
        allocateUBO(swapchain_->imageCount());
        allocateDescriptorPools(swapchain_->imageCount());
        allocateGrahicsCommandBuffers(swapchain_->imageCount());

        graphicsQueue_.reset(new vk::Queue<vk::QueueFamilyType::GRAPHICS>(device_));
        transferQueue_.reset(new vk::Queue<vk::QueueFamilyType::TRANSFER>(device_));
        presentQueue_.reset(new vk::Queue<vk::QueueFamilyType::PRESENT>(device_));

        fence_.reset(new vk::Fence(device_, true));
        imgAvailableSemaphore_.reset(new vk::Semaphore(device_));
        renderFinishedSemaphore_.reset(new vk::Semaphore(device_));
    }

    void uploadBuffers()
    {
        std::array<VkBufferCopy, 1> vertexCopy = {{0, 0, vertices_.size() * sizeof(Vertex)}};
        std::array<VkBufferCopy, 1> indexCopy = {{0, 0, indices_.size() * sizeof(uint32_t)}};

        auto transferCmdBuffer = transferCmdPool_->createCommandBuffer();

        stagingMem_.copyFromHost<Vertex>(
            vertices_.data(), stagingBuffer_->getOffset(), vertices_.size());
        transferCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
            .copyBuffer(*stagingBuffer_, *vertexBuffer_, vertexCopy)
            .end();
        transferQueue_->submit(transferCmdBuffer).waitIdle();

        stagingMem_.copyFromHost<uint32_t>(
            indices_.data(), stagingBuffer_->getOffset(), indices_.size());
        transferCmdBuffer.reset()
            .begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
            .copyBuffer(*stagingBuffer_, *indexBuffer_, indexCopy)
            .end();
        transferQueue_->submit(transferCmdBuffer).waitIdle();
    }

    void renderFrame()
    {
        fence_->waitAndReset();

        uint32_t imageIndex;
        auto res = swapchain_->getNextImage(imageIndex, *imgAvailableSemaphore_);
        if(res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapchain();
            return;
        }

        const auto block = updateMVP();
        uboMem_[imageIndex]->copyFromHost<MatrixBlock>(&block, 0, 1);
        graphicsQueue_->submit(
            graphicsCmdBuffers_[imageIndex],
            {imgAvailableSemaphore_.get()},
            {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            {renderFinishedSemaphore_.get()},
            *fence_);
        presentQueue_->present(*swapchain_, {renderFinishedSemaphore_.get()}, imageIndex);
    }

  private:
    GLFWwindow* window_{nullptr};

    vk::Instance instance_;
    vk::Device device_;
    vk::Memory stagingMem_;
    vk::Memory deviceMem_;

    uint32_t width_{0};
    uint32_t height_{0};

    glm::vec3 centroid_;
    glm::vec3 bboxMin_;
    glm::vec3 bboxMax_;

    vk::Buffer<uint8_t>* stagingBuffer_{nullptr};

    vk::Buffer<Vertex>* vertexBuffer_{nullptr};
    vk::Buffer<uint32_t>* indexBuffer_{nullptr};

    std::unique_ptr<vk::PipelineLayout> graphicsPipelineLayout_{nullptr};
    std::unique_ptr<vk::GraphicsPipeline> graphicsPipeline_{nullptr};
    std::vector<std::unique_ptr<vk::DescriptorPool>> graphicsDescriptorPools_{};

    std::unique_ptr<vk::RenderPass> renderPass_{nullptr};
    std::unique_ptr<vk::Swapchain> swapchain_{nullptr};

    std::unique_ptr<vk::CommandPool<vk::QueueFamilyType::GRAPHICS>> graphicsCmdPool_{nullptr};
    std::unique_ptr<vk::CommandPool<vk::QueueFamilyType::TRANSFER>> transferCmdPool_{nullptr};
    std::unique_ptr<vk::CommandPool<vk::QueueFamilyType::PRESENT>> presentCmdPool_{nullptr};

    std::unique_ptr<vk::Queue<vk::QueueFamilyType::GRAPHICS>> graphicsQueue_{nullptr};
    std::unique_ptr<vk::Queue<vk::QueueFamilyType::PRESENT>> presentQueue_{nullptr};
    std::unique_ptr<vk::Queue<vk::QueueFamilyType::TRANSFER>> transferQueue_{nullptr};

    std::unique_ptr<vk::Fence> fence_{nullptr};
    std::unique_ptr<vk::Semaphore> imgAvailableSemaphore_{nullptr};
    std::unique_ptr<vk::Semaphore> renderFinishedSemaphore_{nullptr};

    std::vector<Vertex> vertices_{};
    std::vector<uint32_t> indices_{};

    std::vector<vk::CommandBuffer<vk::QueueFamilyType::GRAPHICS>> graphicsCmdBuffers_{};

    struct MatrixBlock
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    std::vector<std::unique_ptr<vk::Memory>> uboMem_{};
    std::vector<vk::Buffer<MatrixBlock>*> uboBuffers_;

    MatrixBlock updateMVP()
    {
        const auto diff = glm::abs(bboxMax_ - bboxMin_);
        const float maxDist = std::max(diff.x, std::max(diff.y, diff.z));

        MatrixBlock mvp{glm::mat4{1.0f}, glm::mat4{1.0f}, glm::mat4{1.0f}};
        mvp.model = glm::mat4{1.0f};
        mvp.view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 2.0f * maxDist) + centroid_,
            centroid_,
            glm::vec3{0.0f, 1.0f, 0.0f});
        mvp.proj
            = glm::perspective(glm::radians(45.0f), float(width_) / float(height_), 0.1f, 10.0f);

        return mvp;
    }

    void recreateSwapchain()
    {
        graphicsCmdBuffers_.clear();
        uboBuffers_.clear();
        uboMem_.clear();
        swapchain_->reCreate(width_, height_, VK_FORMAT_B8G8R8A8_SRGB);
        allocateUBO(swapchain_->imageCount());
        allocateDescriptorPools(swapchain_->imageCount());
        allocateGrahicsCommandBuffers(swapchain_->imageCount());
        fence_.reset(new vk::Fence(device_, true));
    }

    void allocateGrahicsCommandBuffers(const uint32_t n)
    {
        if(!graphicsCmdBuffers_.empty())
        {
            graphicsCmdBuffers_.clear();
        }

        graphicsCmdBuffers_ = graphicsCmdPool_->createCommandBuffers(n);
        for(uint32_t i = 0; i < n; ++i)
        {
            auto& graphicsCmdBuffer = graphicsCmdBuffers_[i];
            graphicsCmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT)
                .beginRenderPass(
                    *renderPass_,
                    swapchain_->getFramebuffer(i),
                    VkOffset2D{0, 0},
                    swapchain_->getExtent(),
                    glm::vec4{0.1f, 0.1f, 0.1f, 1.0f})
                .bindGraphicsPipeline(*graphicsPipeline_)
                .setViewport(0, 0, swapchain_->getExtent().width, swapchain_->getExtent().height)
                .setScissor({0, 0}, swapchain_->getExtent())
                .bindGraphicsDescriptorSets(*graphicsPipelineLayout_, *graphicsDescriptorPools_[i])
                .bindVertexBuffer(0, *vertexBuffer_, 0)
                .draw(vertices_.size(), 1, 0, 0)
                .endRenderPass()
                .end();
        }
    }

    void allocateUBO(const uint32_t n)
    {
        for(uint32_t i = 0; i < n; ++i)
        {
            uboMem_.emplace_back(new vk::Memory(
                device_,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            uboBuffers_.emplace_back(
                &uboMem_[i]->createBuffer<MatrixBlock>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 1));
            uboMem_[i]->allocate();
        }
    }

    void allocateDescriptorPools(const uint32_t n)
    {
        for(size_t i = 0; i < n; ++i)
        {
            graphicsDescriptorPools_.emplace_back(new vk::DescriptorPool(
                device_, *graphicsPipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT));
            graphicsDescriptorPools_[i]->bindUniformBuffer(
                0, 0, {uboBuffers_[i]->getHandle(), 0, VK_WHOLE_SIZE});
        }
    }
};

// -------------------------------------------------------------------------------------------------

int main(int, char**)
{
    const uint32_t initWidth = 800;
    const uint32_t initHeight = 600;

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
    GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "Room", nullptr, nullptr);

    // Init Vulkan
    std::unique_ptr<Engine> engine(new Engine(window, initWidth, initHeight));
    engine->readPLY("main/data/room.ply");
    engine->allocateBuffers();
    engine->allocatePipeline();
    engine->uploadBuffers();

    // Main loop
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        engine->renderFrame();
        usleep(10);
    }

    engine.reset(nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}

namespace utils
{
static std::tuple<glm::vec3, glm::vec3, glm::vec3> readPLY(
    const char* filename, std::vector<Vertex>& vertexList, std::vector<uint32_t>& indices)
{
    glm::vec3 bboxMin{std::numeric_limits<float>::max()};
    glm::vec3 bboxMax{std::numeric_limits<float>::lowest()};
    glm::vec3 centroid{0.0f};

    fprintf(stdout, "Reading %s...\n", filename);
    try
    {
        std::unique_ptr<std::istream> file_stream;
        std::vector<uint8_t> byte_buffer;

        file_stream.reset(new std::ifstream(filename, std::ios::binary));
        if(!file_stream || file_stream->fail())
        {
            throw std::runtime_error("Failed to open file");
        }

        // file_stream->seekg(0, std::ios::end);
        file_stream->seekg(0, std::ios::beg);

        ply::PlyFile file;
        file.parse_header(*file_stream);

        auto vertices = file.request_properties_from_element("vertex", {"x", "y", "z"});
        auto colors = file.request_properties_from_element("vertex", {"red", "green", "blue"});
        auto normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"});

        auto faces = file.request_properties_from_element("face", {"vertex_indices"}, 3);

        file.read(*file_stream);

        if(vertices)
        {
            fprintf(stdout, "\tVertices count : %zu\n", vertices->count);
        }
        if(faces)
        {
            fprintf(stdout, "\tFaces count    : %zu\n", faces->count);
        }

        // Update vertices
        vertexList.reserve(vertices->count);
        for(size_t i = 0; i < vertices->count; ++i)
        {
            Vertex v;
            v.position = glm::vec3{
                ((double*) vertices->buffer.get())[3 * i + 0],
                ((double*) vertices->buffer.get())[3 * i + 1],
                ((double*) vertices->buffer.get())[3 * i + 2]};
            v.color = glm::vec3{
                float(colors->buffer.get()[3 * i + 0]) / 255.0f,
                float(colors->buffer.get()[3 * i + 1]) / 255.0f,
                float(colors->buffer.get()[3 * i + 2]) / 255.0f};
            v.normal = glm::vec3{
                ((double*) normals->buffer.get())[3 * i + 0],
                ((double*) normals->buffer.get())[3 * i + 1],
                ((double*) normals->buffer.get())[3 * i + 2]};
            vertexList.push_back(v);

            bboxMin = glm::min(bboxMin, v.position);
            bboxMax = glm::max(bboxMin, v.position);
            centroid += v.position;
        }

        indices.reserve(faces->count);
        for(size_t i = 0; i < faces->count; ++i)
        {
            indices.push_back(((int*) faces->buffer.get())[3 * i + 0]);
            indices.push_back(((int*) faces->buffer.get())[3 * i + 1]);
            indices.push_back(((int*) faces->buffer.get())[3 * i + 2]);
        }
    }
    catch(const std::exception& e)
    {
        fprintf(stderr, "Error reading %s : %s\n", filename, e.what());
        return {};
    }

    fprintf(stdout, "Reading mesh : OK\n");
    fprintf(stdout, "bbox min : %12f %12f %12f\n", bboxMin.x, bboxMin.y, bboxMin.z);
    fprintf(stdout, "bbox max : %12f %12f %12f\n", bboxMax.x, bboxMax.y, bboxMax.z);

    return {centroid / float(vertexList.size()), bboxMin, bboxMax};
}
} // namespace utils