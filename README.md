# Vulkan wrappers library (VKW)

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

This also means not all supported Vulkan features (core or extensions) are yet supported, and
probably not all of them will be.

## Dependencies

VKW needs only two dependencies:

- Vulkan Memory Allocator (VMA): <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>
- Volk: <https://github.com/zeux/volk>

## Building

This is a CMake project, to build it with the samples just do:

```Bash
cmake -DBUILD_SAMPLES=ON -B build && cmake --build build
```

## General principles

This library wraps most common Vulkan principles into C++ objects (Instance, Device, Memory, Buffer,
 Image, ...).

All the Vulkan objects that need to be created/destroyed are wrapped into objects so that creation
and destruction are automatically handled by constructors and destructors.

I try to make in sort that all the objects respect the relationship between original Vulkan objects.

### Vulkan resources

Most of the objects in `vkw` wrap a particular resource: `vkw::Buffer`, `vkw::Image`, `vkw::Device`,
...
Everything that is created using `vkCreate*()`, or `vkAllocate*()`.
The associated handle of all these objects can be retreived using the `getHandle()` method.
Every `vkw` object has the following structure:

- Default empty constructor
- As many constructors with parameters as needed
- No copy and assigment constructors, since it manages Vulkan resources under the hood, it is better
  to manually create as many objects as we have resources than duplicating resources ans making
  deep copies. **VKW does not maintain any reference counter, this logic is let to the application**
  .
- Move constructor and assigmnent implemented: they take care of releasing resources if necessary.
  Take care with them when replacing an already allocated object.
- An set of `init()` methods that use the same parameters as the constructors and init the objects.
- A `clear()` method that destroys all the resources and put the object in the initial state.

After `clear()` is invoked, the object should bt in the same state as if it was created with the
default constructor. A call to `init()` should make it possible to reuse it.
**Other behaviour with `create()` or `init()` should be considered as a bug and will be fixed**.

Some of the objects need to be created in two times, they implement another methods names starting
with `create`. Like `vkw::PipelineLayout::create()`, or `vkw::GraphicsPipeline::create()`.

These `create` methods must be called after the `init()` method and when the initialization of other
parameters is done.

### Other objects

Some `vkw` objects don't wrap any Vulkan resource allocation. In this case, they also implement
default copy constructor and assignment operator.

### Error handling

All methods from `vkw` that construct a Vulkan object will return `false` if the object creation
failed. It is the programer responsibility to check these results. In the case of failure in the
constructor, the error behavior is controlled by the `VKW_CHECK_BOOL_FAIL`, that can throw an
exception, print an error message or don't do anything. An error during the construction of an
object can be detected by checking the result of `isInitialized()` - it will be `false` if a
failure happened.

The `vkw/detail/Utils.hpp` header provide some useful macros to check errors.

In a general way, this library does not offer a substitute to other checks or the use of validation
layers. Application programmers should make sure that they can any non core Vulkan feature.

### Mixing with Vulkan code

It can be necessary to use pure Vulkan code at somes places for various reason:

- A Vulkan feature is not supported by `vkw`
- Comes from some external library
- Any other reason...

`vkw` does not prevent to mix code with other Vulkan code. Anyway, it is necessary to access
Vulkan device functions by using the accessor `vkw::Device::vk()` to get the right function table.

To use Vulkan instance functions before constructing a `vkv` instance,  the static method
`vkw::Instance::initializeVulkan()` must be called before.

In the background, `vkw` uses the Volk loader library: <https://github.com/zeux/volk.git> to load
Vulkan function pointers. There are two advantages for this:

- It uses device tables for function ponters and avoid a dispatch overhead
- All the available extension function pointers are initialized

## Short guide

We will cover the principles here, some basic samples can be found in the `samples/` directory.
To use `vkw`, just add `#include <vkw/vkw.hpp>` into your code. All the wrappers are inside the
`vkw` namespace.

### Instance creation

A Vulkan instance is the first thing ne would like to create before starting. Vulkan instances are
wrapped into the `vkw::Instance` object.

The `vkw::Instance` class will hold a `KkInstance` for us, typical instance creation code will look
like this:

```c++
std::vector<const char*> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
std::vector<vkw::InstanceExtension> instanceExtensions{};
vkw::Instance instance{}; // Instance object not initialized
if(!instance.init(instanceLayers, instanceExtensions))
{
    // Something went wrong here.
}
```

### Using a surface

Surfaces are wrapped by the `vkw::Surface` class. They are just holder objects around a `VkSurface`
handle.

```C++
VkSurface surf = VK_NULL_HANDLE;
// Use any windowing library to create a surface using any previously created instance object...

auto surface = vkw::Surface(std::move(surf)); // surf will be destroyed in vkw::Surface destructor.
```

### Device creation

Vulkan devices are managed by the `vkw::Device` class whose constructor is the following:

```C++
    Device(
        Instance& instance,
        const VkPhysicalDevice& physicalDevice,
        const std::vector<const char*>& extensions,
        const VkPhysicalDeviceFeatures& requiredFeatures,
        const void* pCreateNext = nullptr);
```

By using a `VkPhysicalDevice` handle as a parameter for device creation, the programmer is free to
chose any suitable physical device in the list of the available devices. In case the application
will use some device extensions or enable some specific features, `extensions`, `requiredFeatures`,
and `pCreateNext` must be filled accordingly.

`pCreateNext` will be passed as is in the `pNext` field of the `VkPhysicalDeviceCreateInfo` struct
used to create the `VKDevice` handle.

The static methods `vkw::Device::listSUpportedDevices()` can be use to get a list of the physical
devices that already support the required features.

Let's take for example the case where we would like to create device that supports the
`VK_EXT_mesh_shader` extensions.

We can first have a look to the devices that support the required extension and features:

```C++
vkw::Instance instance{};
// Init instance...

const auto deviceExtensions = std::vector<const char*> {VK_EXT_MESH_SHADER_EXTENSION_NAME};
VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE4_FEATURES;
maintenance4Features.pNext = nullptr;
maintenance4Features.maintenance4 = VK_TRUE;

VkPhysicalDeviceMeshShaderFeaturesExt meshShaderFeatures = {};
meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
meshShaderFeatures.pNext = &maintenance4Features;
meshShaderFeatures.taskShader = VK_TRUE;
meshShaderFeatures.meshShader = VK_TRUE;

const auto supportedDevices =
    vkw::Device::listSupportdeDevices(instance, deviceExtensions, meshShaderFeatures,
        maintenance4Features);
if(supportedDevices.empty())
{
    // No device supports mesh shaders
}

// Use any logic to select an adequate physical device from the available list
// here we just take the first one.
const auto physicalDevice = supportedDevices[0];
vkw::Device device(instance, physicalDevice, deviceExtensions, {}, &meshShaderFeatures);
if(!device.isInitialized())
{
    // Something went wrong but not because of unsupported features.
}
```

### Device queues

Device queues are wrapped in the `vkw::Queue` object. `vkw` does not support specifying the required
device queues we want to use. Instead when creating a logical device, `vkw` will instantiate all the
available queues from this device. Specific queues can be queried by `vkw::Device::getQueues()`, or
`vkw::Device::getPresentQueues` to get the list of all the device queues that support present
operations.

Queues usage is defined with the following flags:

```C++
enum QueueUsageBits
{
    Graphics = 0x01,
    Compute = 0x02,
    Transfer = 0x04,
    SparseBinding = 0x08,
    Protected = 0x10,
    VideoDecode = 0x20,
    VideoEncode = 0x40
};
typedef uint32_t QueueUsageFlags;
```

To get the list of all the device queues that support graphics operations ofr example, just write:

```C++
auto graphicsQueues = device.getQueues(vkw::QueueUsqgeBits::Graphics);
```

It is guaranted that all the queues returned by `vkw::Device::getQueues()` are unique, however, you
may have duplicates from different lists.

As example, a lot of queues from the first queue family support all kinds of operations. Then, it is
more likely that `device.getQueues(vkw::QueueUsageBits::Graphics)[0]` and
`device.getQueues(vkw::QueueUsageBits::Compute)[0]` will eventually point to the same device queue.

If an application needs to work with separate queues, it should take different queues from the same
list or ensure that a returned queue is not already in use.

### Memory management

Memory management in Vulkan is not very trivial and the use of a dedicated `VkDeviceMemory` per
resource is storngly discouraged. Instead any application should use / implement a memory allocator
to manage resources.

`vkw` will simply rely on the Vulkan Memory Alloctor
<https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git> to perform resource
allocations.

`vkw` aims to provide higher level objects than just raw buffer and let applications to specify
which memory types to use.

All the ressources (buffers and images) allocated must specify a memory type defined as follows:

```C++
enum class MemoryType
{
    Device,             ///< Use for data only accessed from the device
    Host,               ///< Use for data that should be mapped on host
    HostStaging,        ///< Use for staging or uniform buffers, permanently mapped
    HostDevice,         ///< Use for large buffers that can be on host if device size is limited
    TransferHostDevice, ///< Used to upload data. Needs to be mapped before using
    TransferDeviceHost  ///< Used for readback. Needs to be mapped before using
};
```

#### Buffers

Buffers are wrapped by the class `vkw::Buffer` as follows:

```C++
template <typename T, MemoryType memType, VkBufferUsageFlags additionalFlags = 0>
class Buffer;
```

Some useful partial specializations are also defined:

```C++
template <typename T, VkBufferUsageFlags additionalFlags = 0>
using DeviceBuffer = Buffer<T, MemoryType::Device, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostBuffer = Buffer<T, MemoryType::Host, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostStagingBuffer = Buffer<T, MemoryType::HostStaging, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostDeviceBuffer = Buffer<T, MemoryType::HostDevice, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using HostToDeviceBuffer = Buffer<T, MemoryType::TransferHostDevice, additionalFlags>;

template <typename T, VkBufferUsageFlags additionalFlags = 0>
using DeviceToHostBuffer = Buffer<T, MemoryType::TransferDeviceHost, additionalFlags>;
```

#### Images

Images are wrapped by the class `vkw::Image` as follows:

```C++
template <MemoryType memType, VkImageUsageFlags additionalFlags = 0>
class Image;
```

Some useful partial specializations are also defined:

```C++
template <VkImageUsageFlags additionalFlags = 0>
using DeviceImage = Image<MemoryType::Device, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostImage = Image<MemoryType::Host, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostStagingImage = Image<MemoryType::HostStaging, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostDeviceImage = Image<MemoryType::HostDevice, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using HostToDeviceImage = Image<MemoryType::TransferHostDevice, additionalFlags>;

template <VkImageUsageFlags additionalFlags = 0>
using DeviceToHostImage = Image<MemoryType::TransferDeviceHost, additionalFlags>;
```

### Resources binding

#### Pipeline layout

Resource binding for shaders follows the same relationships as in the Vulkan specification. An
application defines a number of descriptor set layout objects and use them to build pipeline layouts
and allocate descriptor set objects.

Descriptor set layouts are stored in the `vkw::DescriptorSetLayout` class. `vkw` supports the
following descriptor types:

```C++
enum class DescriptorType : uint32_t
{
    Sampler = 0,
    CombinedImageSampler = 1,
    SampledImage = 2,
    StorageImage = 3,
    UniformTexelBuffer = 4,
    StorageTexelBuffer = 5,
    UniformBuffer = 6,
    StorageBuffer = 7,
    UniformBufferDynamic = 8,
    StorageBufferDynamic = 9,
    InputAttachment = 10,
    AccelerationStructure = 11
};
```

Let's suppose we are using a compute shader that looks like this:

```GLSL
layout(set = 0, binding = 1) buffer Buffer0 {float buffer0[0]};
layout(set = 0, binding = 0) buffer Buffer1 {float buffer1[0]};

layout(set = 1, binding = 0, rgba32f) uniform image2D image;
```

Then somewhere in the application we need to specify two descriptor set layouts:

```C++
vkw::Device& = getDevice(); // Device initialized somewhere else

vkw::DescriptorSetLayout layout0{device};
layout0.addBinding0<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 0)
       .addBinding0<vkw::DescriptorType::StorageBuffer>(VK_SHADER_STAGE_COMPUTE_BIT, 1)
       .create();

vkw::DescriptorSetLayout layout1{device};
layout1.addBinding0<vkw::DescriptorType::StorageImage>(VK_SHADER_STAGE_COMPUTE_BIT, 0)
        .create();

vkw::PipelineLayout pipelineLayout{device, layout0, layout1};
pipelineLayout.create();
```

The base class is `vkw::PipelineLayout`, typical usage will be:

```C++
const uint32_t setCount = 2;
vkw::PipelineLayout pipelineLayout(device, setCount);
pipelineLayout.getDescriptorSetLayout(0)
    .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 0)
    .addStorageImageBinding(VK_SHADER_STAGE_COMPUTE_BIT, 1);
pipelineLayout.getDescriptorSetLayouts(1)
    .addUniformBufferBinding(VK_SHADER_STAGE_COMPUTE_BIT, 2);
pipelineLayout.create();
```

#### Descriptor pool and descriptor sets

We can then allocate some descriptor sets that will contain the resource bindings. The base class to
create descriptor sets will be `vkw::DescriptorPool`.

```C++
vkw::DescriptorPool descriptorPool(device, maxSetCount, poolSizes);
auto descriptorSets = descriptorPool.allocateDescriptorSets(descriptorSetLayout, count);
for(size_t i = 0; i < count; ++i)
{
    auto& descriptorSet = descriptorSets[i];
    descriptorSet.bindStorageBuffer(s0, buffers0[i], buffers1[i]);
    // ...
}
```

### Compute pipeline

Once a pipeline layout is created, a compute pipeline object can be created. It needs a link to a
binary `.spv` file containing the SPIR-V code of a compute shader. Specification constants can
optionally be added.

Typical usage of a compute pipeline will be:

```c++
vkw::ComputePipeline computePipeline(device, shaderBinaryPath);
computePipeline.addSpec<uint32_t>(val0)
               .addSpec<uint32_t>(val1);
               .createPipeline(pipelineLayout);
```

### Graphics pipeline

Graphics pipelines are more complex than compute pipelines since they also support all the graphics
pipeline stages.

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

The `vkw::GraphicsPipeline` class will instantiate the graphics pipeline with some default
parameters for all the structs that should be overriden by the application. Each of the pipeline
creation stages can be accessed via:

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

### RenderPass

The `vkw::RenderPass` class maps the Vulkan equivalent, it just defines the attachments used and how
to deal with them. `vkw` also supports dynamic rendering if a graphics pipeline is constructed
without a renderpass reference.

### Swapchain

Swapchains need to be created carefully. Swapchain are managed by the `vkw::Swapchain` class.
There are two constructors depending on if a renderpass is used. If no renderpass is used, no
framebuffer will be created.

```c++
    explicit Swapchain(
        Surface& surface,
        Device& device,
        RenderPass& renderPass,
        const uint32_t w,
        const uint32_t h,
        const uint32_t maxImageCount,
        const VkFormat colorFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});

    explicit Swapchain(
        Surface& surface,
        Device& device,
        const uint32_t w,
        const uint32_t h,
        const uint32_t maxImageCount,
        const VkFormat colorFormat,
        const VkImageUsageFlags usage,
        const VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        const std::vector<uint32_t>& queueFamilyIndices = {});
```

Swapchains are created at their construction and manage internally a list of swapchain images and
framebuffers (if a renderpass is used) for the rendering. The number of swapchain images can be
get with `vkw::Swapchain::imageCount()`.
**The sawpahchain API will probably be updated soon**

To get the next available swapchain image, just call: `vkw::Swapchain::getNextImage()`.

If the window is resized, the swapchain can be re created by calling `vkw::Swapchain::reCreate()`.

```c++
bool reCreate(
    const uint32_t w,
    const uint32_t h,
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    const std::vector<uint32_t>& queueFamilyIndices = {});
```

See `samples/IGraphicsSample.cpp` for an example of swapchain management.

### Command buffers

Command buffers are allocated via the `vkw::CommandPool` class.

```c++
vkw::CommandPool cmdPool(device);
auto cmdBuffer = cmdPool.createCommandBuffers(count);
```

The `vkw::CommandBuffer` class wraps all the Vulkan command functions `vkCmd*()`.
To record a command buffer, simply do:

```c++
cmdBuffer.begin(VK_COMMAND_BUFFER_ONE_TIME_SUBMIT_BIT)
         .bindComputePipeline(...)
         .bindComputeDescriptorSets(pipelineLayout,...)
         .pushConstants(...)
         .dispatch(gridSizeX, gridSizeY, gridSizeZ)
         .pipelineBarrier(...)
         .beginRenderPass()
         .endRenderPass()
         // And other graphics stuff...
         .end();
```