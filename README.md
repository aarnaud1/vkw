# Description

This repository contains some useful Vulkan wrapper functions to hide a bit of the Vulkan
boilerplate code.

This is a C++17 library that aims to add a small abstraction over Vulkan API to remove some
redudancies.
 This is not intended to be a render engine, and I tried to make as minimum abstractions possible to
 keep it close to the Vulkan API.
 In order to acheive this, this library relies on the following:
 - RAII: each Vulkan object is wrapped in a C++ class and resource freeing is done when the object
    is destroyed. Then it is unnecessary to make a call to any `vkDestroy*()` function.
- C++ features: when possible, use template or other C++ features that avoid to duplicate things
- Default parameters: set some params to default when not used to make it more readable
- Some design choices: I tried to minimize as much as possible this but you cannot really provide an
  abstraction over Vulkan without making some considerations and making some choices. This whole
  library is a tradeoff between the full verbosity of Vulkan and some assumptions that make life
  easier.

This library is a reflect of my knowledge of Vulkan and some usages that could be made of it. It was
originally made to provide some useful functions to quickly test some Vulkan features, but would
probably not be suitable for a professional use. It is however very handy to create some quick
Vulkan samples or applications.

This library is **still under development** and some parts of the API may change depending on some
identified usages.

This also means not all supported Vulkan features (core or extensions) are yet supported, and probably not all of them will be.

## About VK extensions

These wrappers support a few Vulkan extensions, each extension support will be added progressively
when needed. The nature of these wrappers make it difficult to allow any extension like this, so I
chose to support only a set of defined Vulkan extensions for now. I will be in the future how to
handle extensions in a better way.

# General principles

This library wraps most common Vulkan principles into C++ objects (Instance, Device, Memory, Buffer,
 Image, ...).

All the Vulkan objects that need to be created/destroyed are wrapped into objects so that creation
and destruction are automatically handled by constructors and destructors.

I try to make in sort that all the objects respect the relationship between original Vulkan objects.

There are two types of objects: independant objects (most common) and managed objects.

Each `vkw` object wraps a Vulkan object that can be retrived with the `getHandle()` method.

## Independant objects

They are created independently from each others, destructor handles the freeing of
their resources.

They follow the current rules:

- Default empty constructor
- As many constructors with parameters as needed
- No copy and assigment constructors, since it manages Vulkan resources under the hood, it is better
  to manually create as many objects as we have resources than duplicating resources ans making
  deep copies.
- Move constructor and assigmnent implemented: they take care of releasing resources if necessary.
  Take care with them when replacing an already allocated object.
- An set of `init()` methods that use the same parameters as the constructors and init the objects.
- A `clear()` method that destroys all the resources and put the object in the initial state.

After `clear()` is invoked, the object should bt in the same state as if it was created with the default constructor. A call to `init()` should make it possible to reuse it.
**Other behaviour with `create()` or `init()` should be considered as a bug and will be fixed**.

Some of the objects need to be created in two times, they implement another methods names starting
with `create`. Like `vkw::PipelineLayout::create()`, or `vkw::GraphicsPipeline::create()`.

These `create` methods must be called after the `init()` method and when the initialization of other parameters is done.

## Managed objects

Managed objects, such as `Memory`, `Buffer`, `Queue` which are created and manages by a parent
object. These objects can be copied moved and destroyed without consequences.
**Managed objects resources are freed when the parent object is destroyed**.

## Error handling

In case of errors during the constructor, an exception is thrown and the description of the problem is written in the error output.
In case of an error during the initialization, the `init()` method return `false` and calls the `clear()` method so no memory leak should happen.

Other functions can throw an exception when allocating Vulkan objects. Error handling is basic, and it should not be assumed that the library will do an intensive error check.

In a general way, this library does not offer a substitute to other checks or the use of validation layers. Application programmers should make sure that they can use some Vulkan features. This library does not prevent to use the Vulkan query functions before creating objects to check these features are supported.

# Short guide

We will cover the principles here, some basic samples can be found in the `samples/` directory.
To use `vkw`, just add `#include <vkWrappers/wrappers.hpp>` into your code. All the wrappers are inside the `vkw` namespace.

## Instance creation

A Vulkan instance is the first thing ne would like to create before starting. Vulkan instances are wrapped into the `vkw::Instance` object.

The `vkw::Instance` class will hold a `vkInstance` for us, typical instance creation code will look like this:
```c++
std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
std::vector<vkw::InstanceExtension> instanceExtensions{};
vkw::Instance instance(instanceLayers, instanceExtensions);
```

See `wrappers/extensions/InstanceExtensions` for the detailed list of supported instance extensions.

`vkw` does not provide a way to specify a `vkApplicationInfo` struct, same for the `vkInstanceCreateInfo` struct, that is made to use the lates Vulkan version.

## Using a surface

If the application is used to use a display surface, the adequate instance extensions must be enabled and a `vkSurface` object must be provided. The `vkw::Instance` will handle the descruction of the surface in its destructor.

Surface creation has been externalized because it is dependant of the platform and the window library used.

To create an instance with GLFW on Linux just do:
```c++
const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<vkw::InstanceExtension> instanceExts = {vkw::SurfaceKhr, vkw::XcbSurfaceKhr};
vkw::Instance instance(instanceLayers, instanceExts);
glfwCreateWindowSurface(instance.getHandle(), window, nullptr, &surface);
instance.setSurface(std::move(surface));
```

## Device creation

The device creation follows the same rules of the instance creation:
```c++
const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
    = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
const VkPhysicalDeviceFeatures requiredFeatures{};
const std::vector<vkw::DeviceExtension> deviceExts = {vkw::SwapchainKhr};
vkw::Device device(instance, deviceExts, requiredFeatures, compatibleDeviceTypes);
```

Take care to add the required extensions and features. `vkw` won't perform too intensive code validation, it will be useful to enable validation layers while debugging.

The application needs to provide a list of physical device types to use with an order of preferences. When creating the device, `vkw` will select a physical device that has all the required features following this order of preferences.

### Device queues

When creating a logical device, `vkw` will instantiate all the available queues from this device. Specific queues can be queried by `vkw::Device::getQueues()`. It will take a mask indicating the operations that have to be supported, and will return the list of all de device queues supporting it.

```c++
// Returns the list of all the queues that support all four graphics, compute, transfer and
// compute operations.
auto allOperationsQueues = device.getQueues(
    vkw::QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT
    | vkw::QueueUsageBits::VKW_QUEUE_COMPUTE_BIT
    | vkw::QueueUsageBits::VKW_QUEUE_TRANSFER_BIT
    | vkw::QueueUsageBits::VKW_QUEUE_PRESENT_BIT);

// Returns the list of all the device queues that support graphics operations.
// /!\ some of these queues are the same as in allOperationQueues.
auto graphicsQueues = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT);
```

It is guaranted that all the queues returned by `vkw::Device::getQueues()` are unique, however, you may have duplicates from different lists.

As example, a lot of queues from the first queue family support all kinds of operations. Then, it is more likely than `device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT)[0]` and `device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_COMPUTE_BIT)[0]` will eventually point to the same device queue.

If an application needs to work with separate queues, it should take different queues from the same list or ensure that a returned queue is not already in use.

## Memory management

### Description

Memory management is a big topic and it is difficult to make something generic since it is really dependant of the application to be developed and the hardware used.

This is probably one of the main limitation for the use of `vkw` in bigger applications.
I tried to make something that would fit the basic use cases.

Memory management is made with from one side the `vkDeviceMemory` that represents a device memory allocation. And `VkBuffer` and `vkImage` objects, that represent actually used resources and that must be bound to a zone of an allocated memory.

The Vulkan specification however doesn't prevent to bind multiple buffers to a same memory region, however, it seems this can be easily avoided and not so much necessary.

One choice in `vkw` is that buffer will always be bound to separate regions in a memory.

We cannot unfortunately create one memory object each time we create some image or buffer. This is even discouraged and the amount of memory allocations is limited. We should really work with huge chunks of memory that will be used to store multiple buffers and images.

### Creating memory and resources

The design of `vkw` is then to implement a `vkw::Memory` object that manages to allocate a chunk of device memoory, and creates bound resources to it.

Each memory can create `Buffer` and `Image` objects and than a call to `vkw::Memory::allocate()` will allocate the needed memory. The `vkw::Memory` class also manage alignment so no need to take care of it, offsets are added between two buffers to match the akignment specified in their memory requirements.

The basic usage of `vkw::Memory` is the following:
```c++
vkw::Memory deviceMem(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
auto buffer0 = deviceMem.createBuffer<float>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, count0);
auto buffer1 = deviceMem.createBuffer<float>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, count1);
auto image0 = deviceMem.createImage(VK_IMAGE_TYPE_2D, format, extent, VK_IMAGE_USAGE_STORAGE_IMAGE_BIT);
deviceMem.allocate(); // Call it afterhaving created all the managed buffer and images
```

When finding for some adequate memory type and heap, the `vkw::Memory` constructor will look to a memory that maps exactly the memory property flags, or at least that contains them.
A more advanced memory search should be implemented in the future.

**/!\ There is no protection against memory leaks when not getting the result of `vkw::Memory::createBuffer()` or `vkw::Memory::createImage()`. If not affected to a variable, these buffers/images cannot be retrieved but space will be allocated for them in memory.**

For example, if we do something like this:
```c++
vkw::Memory deviceMem(device, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
deviceMem.createBuffer<float>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, count);
deviceMem.createBuffer<float>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, count);
deviceMem.createBuffer<float>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, count);
deviceMem.allocate();
```

`deviceMem` will allocate `3 * count * sizeof(float)` bytes that will be freed only when `deviceMem` will be destroyed. This issue should be addressed in the future.

## Resources binding

Resource binding for shaders follows the same relationships as in the Vulkan specification. This can be separated into two separate classes: `vkw::PipelineLayout` which defines the number and type of descriptors a given pipeline will have, and `vkw::DescriptorPool` that will allocate descriptor sets with the actual bindings.

### Pipeline layout

The base class is `vkw::PipelineLayout`, typical usage will be:
```c++
const uint32_t setCount = 2;
vkw::PipelineLayout pipelineLayout(device, setCount);
pipelineLayout.getDescriptorSetLayout(0)
    .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0)
    .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1);
pipelineLayout.getDescriptorSetLayouts(1)
    .addUniformBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 2);
pipelineLayout.create();
```

### Descriptor pool and descriptor sets

We can then allocate some descriptor sets that will contain the resource bindings. The base class to create descriptor sets will be `vkw::DescriptorPool`. It must be constructed with the maximum number of bindings and descriptor sets that will be output.

```c++
vkw::DescriptorPool descriptorPool(device, maxSetCount, maxDescriptorCount);
auto descriptorSets = descriptorPool.allocateDescriptorSets(descriptorSetLayout, count);
for(size_t i = 0; i < count; ++i)
{
    auto& descriptorSet = descriptorSets[i];
    descriptorSet.bindStorageBuffer(0, buffers0[i]);
    descriptorSet.bindStorageBuffer(1, buffers1[i]);
    // ...
}
```
Only a subset of descriptor types can be used for now, this list will be extended in the future.

**/!\ There is no protection against memory leaks when not getting the result of `vkw::DescriptorPool::allocateDescriptorSets()` or `vkw::DescriptorPool::allocateDescriptorSet()`. If not affected to a variable, these descriptor sets cannot be retrieved but space will be allocated for them in memory.**

## Compute pipeline

Once a pipeline layout is created, a compute pipeline object can be created. It needs a link to a binary `.spv` file containing the SPIR-V code of a compute shader. Specification constants can optionally be added.

Typical usage of a compute pipeline will be:
```c++
vkw::ComputePipeline computePipeline(device, shaderSource);
computePipeline.addSpec<uint32_t>(val0)
               .addSpec<uint32_t>(val1);
               .createPipeline(pipelineLayout);
```

## Graphics pipeline

Graphics pipelines are more complex than compute pipelines since they also support all the graphics pipeline stages.

Basic usage can be:
```c++
struct Vertex{};
vkw::GraphicsPipeline graphicsPipeline(device);
graphicsPipeline.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderSource);
graphicsPipeline.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShaderSource);
graphicsPipeline.addVertexBinding(0, sizeof(Vertex))
                .addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
                .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col));
graphicsPipeline.createPipeline(renderPass, pipelineLayout);
```

The `vkw::GraphicsPipeline` class will instantiate the graphics pipeline with some default parameters for all the structs that should be overriden by the application. Each of the pipeline creation stages can be accessed via:
```c++
    auto& GraphicsPipeline::viewports();
    auto& GraphicsPipeline::scissors();
    auto& GraphicsPipeline::colorBlendAttachmentStates();
    auto& GraphicsPipeline::inputAssemblyStateInfo();
    auto& GraphicsPipeline::tesselationStateInfo();
    auto& GraphicsPipeline::rasterizationStateInfo();
    auto& GraphicsPipeline::multisamplingStateInfo();
    auto& GraphicsPipeline::depthStencilStateInfo();
    auto& GraphicsPipeline::colorBlendStateInfo();
    auto& GraphicsPipeline::dynamicStateInfo();
```

Input assembly will be generated from what have been passed to:
```c++
GraphicsPipeline& GraphicsPipeline::addVertexBinding();
GraphicsPipeline& GraphicsPipeline::addVertexAttribute();
```

## RenderPass

The `vkw::RenderPass` class maps the Vulkan equivalent, it just defines the attachments used and how to deal with them. Basic example fo a render pass object can be seen in `samples/Triangle.cpp`.

## Swapchain

Swapchains need to be created carefully. Swapchain are managed by the `vkw::Swapchain` class. There are two constructors: one for swapchains with color only and one for swapchains with depth buffer.

```c++
explicit Swapchain(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat colorFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    const std::vector<uint32_t>& queueFamilyIndices = {});

explicit Swapchain(
    Instance& instance,
    Device& device,
    RenderPass& renderPass,
    const uint32_t w,
    const uint32_t h,
    const VkFormat colorFormat,
    const VkFormat depthStencilFormat,
    const VkImageUsageFlags usage,
    const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    const std::vector<uint32_t>& queueFamilyIndices = {});
```

Swapchains are created at their construction and manage internally a list of swapchain images and framebuffers for the rendering. The number of swapchain images can be get with `vkw::Swapchain::imageCount()`. Swapchains are created with 3 images (triple buffering).

To get the next available swapchain image, just call: `vkw::Swapchain::getNextImage()`.

If the window is resized, the swapchain can be re created by calling `vkw::Swapchain::reCreate()`.
```c++
bool reCreate(
    const uint32_t w,
    const uint32_t h,
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    const std::vector<uint32_t>& queueFamilyIndices = {});
```

See `samples/Triangle.cpp` for an example of swapchain management.

## Command buffers

Command buffers are allocated via the `vkw::CommandPool` class.
```c++
vkw::CommandPool cmdPool(device);
auto cmdBuffer = cmdPool.createCommandBuffers(count);
```

The `vkw::CommandBuffer` class wraps all the Vulkan command functions `vkCmd*()`.
To record a command buffer, simply do/
```c++
cmdBuffer.begin(VK_COMMAND_BUFFER_ONE_TIME_SUBMIT_BIT)
         .bindComputePipeline(...)
         .bindComputeDescriptorSets(pipelineLayout,...)
         .pushCOnstants(...)
         .dispatch(gridSizeX, gridSizeY, gridSizeZ)
         .pipelineBarrier(...)
         .beginRenderPass()
         .endRenderPass()
         // And other graphics stuff...
         .end();
```

# Putting it all together

Lets see with a simple triangle how things are going.
First include all the necessary headers/
```c++
#include <glm/glm.hpp>
#include <vkWrappers/wrappers.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
```

Define an array of vertices to display, and the dislay color format.
```c++
struct Vertex
{
    glm::vec2 pos;
    glm::vec3 col;
};
const std::vector<Vertex> vertices
    = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
       {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
       {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

static constexpr VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
```

Then we will initialize GLFW in our min function and the `vkw` library, we won't use a resizable window here.
```c++
const uint32_t initWidth = 800;
const uint32_t initHeight = 600;
VkSurfaceKHR surface = VK_NULL_HANDLE;

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
glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "Triangle", nullptr, nullptr);

// Init Vulkan
const std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
// We use UNIX extensions here
const std::vector<vkw::InstanceExtension> instanceExts
    = {vkw::DebugUtilsExt, vkw::SurfaceKhr, vkw::XcbSurfaceKhr};
vkw::Instance instance(instanceLayers, instanceExts);
glfwCreateWindowSurface(instance.getHandle(), window, nullptr, &surface);
instance.setSurface(std::move(surface));

const std::vector<VkPhysicalDeviceType> compatibleDeviceTypes
    = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU};
const std::vector<vkw::DeviceExtension> deviceExts = {vkw::SwapchainKhr};
vkw::Device device(instance, deviceExts, {}, compatibleDeviceTypes);

// Get a graphics and a present queue
auto graphicsQueue = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_GRAPHICS_BIT)[0];
auto presentQueue = device.getQueues(vkw::QueueUsageBits::VKW_QUEUE_PRESENT_BIT)[0];
```

Then we will allocate a buffer for out triangle vertices. We will also need to transfer out vertices from CPU to GPU. In this example, we only have 3 vertices, we will use only one memory type that will be visible by the host. In more complex applications, we will prefer to use some huge device allocated memory allocations, and manage host - device transfers using smaller staging buffers.

```c++
vkw::Memory vertexMemory(device,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
                        | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                        | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
auto vertexBuffer =
    vertexMemory.createBuffer<Vertex>(VK_MEMORY_USAGE_VERTEX_BUFFER_BIT, vertices.size());
vertewMemory.create();

// Transfer from CPU to GPU
vertexMemory.copyFromHost<Vertex>(vertices.data(), vertexBuffer.getMemOffset(), vertices.size());
```

Then we will create the graphics objects, lets start with a renderPass object that will be defined as follows in our case:
```c++
vkw::RenderPass renderPass(device);
renderPass
    .addColorAttachment(
        colorFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_SAMPLE_COUNT_1_BIT)
    .addSubPass({0})
    .addSubpassDependency(
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
    .create();
```

Then we will need a basic pipeline layout here/
```c++
vkw::PipelineLayout pipelineLayout(device, 0);
pipelineLayout.create();
```

We need a basic vertex shader:
```glsl
#version 450 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 col;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 vertexColor;

void main()
{
    gl_Position = vec4(pos, 0.0f, 1.0f);
    vertexColor = col;
}
```

And a basic fragment shader:
```glsl
#version 450 core

layout(location = 0) in vec3 vertexColor;

layout(location = 0) out vec4 fragColor;

void main() { fragColor = vec4(vertexColor, 1.0f); }
```

We can then create a graphics pipeline:
```c++
vkw::GraphicsPipeline graphicsPipeline(device);
graphicsPipeline.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, "output/spv/triangle_vert.spv");
graphicsPipeline.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, "output/spv/triangle_frag.spv");
graphicsPipeline.addVertexBinding(0, sizeof(Vertex))
    .addVertexAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos))
    .addVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, col));
graphicsPipeline.createPipeline(renderPass, pipelineLayout);
```

Create a swapchain:
```c++
vkw::Swapchain swapchain(
    instance,
    device,
    renderPass,
    initWidth,
    initHeight,
    colorFormat,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
```

Create a command pool and a command buffer:
```c++
vkw::CommandPool graphicsCmdPool(device, graphicsQueue);

auto recordCommandBuffer
    = [&](auto& cmdBuffer, const uint32_t i, const uint32_t w, const uint32_t h) {
          cmdBuffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
              .beginRenderPass(
                  renderPass,
                  swapchain.getFramebuffer(i),
                  VkOffset2D{0, 0},
                  VkExtent2D{w, h},
                  glm::vec4{0.1f, 0.1f, 0.1f, 1.0f})
              .bindGraphicsPipeline(graphicsPipeline)
              .setViewport(0.0f, 0.0f, float(w), float(h))
              .setScissor({0, 0}, {w, h})
              .setCullMode(VK_CULL_MODE_NONE)
              .bindVertexBuffer(0, vertexBuffer, 0)
              .draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0)
              .endRenderPass()
              .end();
      };
auto cmdBuffer = graphicsCmdPool.createCommandBuffer();
```

Main loop:
```c++
#define MAX_FRAMES_IN_FLIGHT 3

uint32_t currentFrame = 0;

std::vector<vkw::Semaphore> imageAvailableSemaphores;
imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
{
    imageAvailableSemaphores[i].init(device);
}

std::vector<vkw::Semaphore> renderFinishedSemaphores{};
renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
{
    renderFinishedSemaphores[i].init(device);
}

std::vector<vkw::Fence> fences;
fences.resize(MAX_FRAMES_IN_FLIGHT);
for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
{
    fences[i].init(device, true);
}

while(!glfwWindowShouldClose(window))
{
    glfwPollEvents();

    auto& fence = fences[currentFrame];
    fence.wait();

    uint32_t imageIndex;
    swapchain.getNextImage(imageIndex, imageAvailableSemaphores[currentFrame], UINT64_MAX);

    auto& cmdBuffer = commandBuffers[currentFrame];
    cmdBuffer.reset();
    recordCommandBuffer(
        cmdBuffer, imageIndex, swapchain.getExtent().width, swapchain.getExtent().height);

    graphicsQueue.submit(
        cmdBuffer,
        std::vector<vkw::Semaphore*>{&imageAvailableSemaphores[currentFrame]},
        {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        std::vector<vkw::Semaphore*>{&renderFinishedSemaphores[currentFrame]},
        fence);
    resentQueue.present(
        swapchain,
        std::vector<vkw::Semaphore*>{&renderFinishedSemaphores[currentFrame]},
        imageIndex);
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// End of main function
device.waitIdle();

glfwDestroyWindow(window);
glfwTerminate();

return EXIT_SUCCESS;
```