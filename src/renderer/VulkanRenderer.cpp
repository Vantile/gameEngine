#include <renderer/VulkanRenderer.h>

#include <assert.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <renderer/Mesh.h>
#include <renderer/PipelineBuilder.h>
#include <renderer/RenderPoint.h>
#include <renderer/Shader.h>
#include <renderer/Vertex.h>
#include <renderer/VulkanUtils.h>
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

void VulkanRenderer::Run()
{
    if (m_ResizeRequested)
    {
        ResizeSwapchain();
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Randomize"))
    {
        if (ImGui::Button("Randomize"))
        {
            for (std::shared_ptr<Mesh> mesh : m_DrawContext.m_DrawMeshes)
            {
                mesh->Randomize();
                UpdateMeshBuffers<Vertex, uint32_t>(mesh->GetVertices(), mesh->GetIndices(), mesh->GetVertexBuffer(), mesh->GetIndexBuffer(), mesh->GetVertexBufferAddress());
            }
        }
    }
    ImGui::End();

    ImGui::Render();

    Draw();
}

void VulkanRenderer::ProcessSDLEvent(SDL_Event& e)
{
    ImGui_ImplSDL3_ProcessEvent(&e);
}

void VulkanRenderer::Init()
{
    // only one engine initialization is allowed with the application.
    if (m_IsInitialized)
    {
        assert(false);
        return;
    }

    // We initialize SDL and create a window with it.
    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    m_Window = SDL_CreateWindow(
        "Engine",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowFlags);

    InitVulkan();
    InitSwapchain();
    InitCommands();
    InitSyncStructures();
    InitDescriptors();
    InitPipelines();
    InitImgui();
    InitDefaultData();
    //InitCamera();

    // Everything went fine
    m_IsInitialized = true;
}

void VulkanRenderer::Cleanup()
{
    if (m_IsInitialized)
    {
        vkDeviceWaitIdle(m_Device);

        for (int i = 0; i < FRAME_OVERLAP; i++)
        {
            vkDestroyCommandPool(m_Device, m_Frames[i].m_CommandPool, nullptr);

            vkDestroyFence(m_Device, m_Frames[i].m_RenderFence, nullptr);
            vkDestroySemaphore(m_Device, m_Frames[i].m_RenderSemaphore, nullptr);
            vkDestroySemaphore(m_Device, m_Frames[i].m_SwapchainSemaphore, nullptr);

            m_Frames[i].m_FrameDeletionQueue.Flush();
        }

        m_MainDeletionQueue.Flush();

        DestroySwapchain();

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyDevice(m_Device, nullptr);

        vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
        vkDestroyInstance(m_Instance, nullptr);
        SDL_DestroyWindow(m_Window);
    }

    m_IsInitialized = false;
}

void VulkanRenderer::InitVulkan()
{
    vkb::InstanceBuilder builder;

    auto returnValue = builder.set_app_name("Vulkan App")
        .request_validation_layers(true)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance instance = returnValue.value();
    m_Instance = instance.instance;
    m_DebugMessenger = instance.debug_messenger;

    SDL_Vulkan_CreateSurface(m_Window, m_Instance, nullptr, &m_Surface);

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{ instance };
    vkb::PhysicalDevice physicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features13)
        .set_required_features_12(features12)
        .set_surface(m_Surface)
        .select().value();

    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    m_Device = vkbDevice.device;
    m_ChosenGPU = physicalDevice.physical_device;

    m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    m_GraphicsQueueIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    m_MemoryManager.InitAllocator(m_ChosenGPU, m_Device, m_Instance);

    m_MainDeletionQueue.PushFunction([&]() {
        m_MemoryManager.DestroyAllocator();
    });
}

void VulkanRenderer::InitSwapchain()
{
    CreateSwapchain(m_WindowExtent.width, m_WindowExtent.height);

    VkExtent3D drawImageExtent = {
        m_WindowExtent.width,
        m_WindowExtent.height,
        1
    };

    m_DrawImage.m_ImageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    m_DrawImage.m_ImageExtent = drawImageExtent;

    // TODO: Do I need all of these?
    VkImageUsageFlags drawImageUsages{};
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
    drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkImageCreateInfo imageInfo = VulkanUtils::Image::ImageCreateInfo(m_DrawImage.m_ImageFormat, drawImageUsages, drawImageExtent);

    // Allocate draw image from GPU local memory
    VmaAllocationCreateInfo imageAllocationInfo{};
    imageAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    imageAllocationInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_MemoryManager.AllocateImage(&imageInfo, &imageAllocationInfo, &m_DrawImage.m_Image, &m_DrawImage.m_Allocation);

    // Build an image-view for the draw image to use for rendering
    VkImageViewCreateInfo viewInfo = VulkanUtils::Image::ImageViewCreateInfo(m_DrawImage.m_ImageFormat, m_DrawImage.m_Image, VK_IMAGE_ASPECT_COLOR_BIT);
    VULKAN_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_DrawImage.m_ImageView));

    m_DepthImage.m_ImageFormat = VK_FORMAT_D32_SFLOAT;
    m_DepthImage.m_ImageExtent = drawImageExtent;
    VkImageUsageFlags depthImageUsages{};
    depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageCreateInfo depthImageInfo = VulkanUtils::Image::ImageCreateInfo(m_DepthImage.m_ImageFormat, depthImageUsages, drawImageExtent);
    m_MemoryManager.AllocateImage(&depthImageInfo, &imageAllocationInfo, &m_DepthImage.m_Image, &m_DepthImage.m_Allocation);

    VkImageViewCreateInfo depthViewInfo = VulkanUtils::Image::ImageViewCreateInfo(m_DepthImage.m_ImageFormat, m_DepthImage.m_Image, VK_IMAGE_ASPECT_DEPTH_BIT);
    VULKAN_CHECK(vkCreateImageView(m_Device, &depthViewInfo, nullptr, &m_DepthImage.m_ImageView));

    m_MainDeletionQueue.PushFunction([=]() {
        vkDestroyImageView(m_Device, m_DrawImage.m_ImageView, nullptr);
        m_MemoryManager.DestroyImage(m_DrawImage.m_Image, m_DrawImage.m_Allocation);

        vkDestroyImageView(m_Device, m_DepthImage.m_ImageView, nullptr);
        m_MemoryManager.DestroyImage(m_DepthImage.m_Image, m_DepthImage.m_Allocation);
    });
}

void VulkanRenderer::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolInfo = VulkanUtils::Command::CommandPoolCreateInfo(m_GraphicsQueueIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (uint32_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        VULKAN_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_Frames[i].m_CommandPool));

        VkCommandBufferAllocateInfo commandAllocInfo = VulkanUtils::Command::CommandBufferAllocateInfo(m_Frames[i].m_CommandPool, 1);
        VULKAN_CHECK(vkAllocateCommandBuffers(m_Device, &commandAllocInfo, &m_Frames[i].m_CommandBuffer));
    }

    VULKAN_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_ImmediateCommandPool));
    VkCommandBufferAllocateInfo immediateAllocInfo = VulkanUtils::Command::CommandBufferAllocateInfo(m_ImmediateCommandPool, 1);
    VULKAN_CHECK(vkAllocateCommandBuffers(m_Device, &immediateAllocInfo, &m_ImmediateCommandBuffer));
    m_MainDeletionQueue.PushFunction([=]() {
        vkDestroyCommandPool(m_Device, m_ImmediateCommandPool, nullptr);
    });
}

void VulkanRenderer::InitSyncStructures()
{
    VkFenceCreateInfo fenceCreateInfo = VulkanUtils::Sync::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = VulkanUtils::Sync::SemaphoreCreateInfo();
    for (uint32_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        VULKAN_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_Frames[i].m_RenderFence));
        VULKAN_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_RenderSemaphore));
        VULKAN_CHECK(vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_Frames[i].m_SwapchainSemaphore));
    }

    VULKAN_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_ImmediateFence));
    m_MainDeletionQueue.PushFunction([=]() {
        vkDestroyFence(m_Device, m_ImmediateFence, nullptr);
    });
}

void VulkanRenderer::InitDescriptors()
{
    // Descriptor pool to hold 10 sets with 1 image each set
    std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };

    constexpr uint32_t maxDescriptorSets = 10;
    m_GlobalDescriptorAllocator.Init(m_Device, maxDescriptorSets, sizes);

    // Make the descriptor set layout for compute draw
    DescriptorLayoutBuilder drawImageDescriptorLayoutBuilder;
    drawImageDescriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    m_DrawImageDescriptorLayout = drawImageDescriptorLayoutBuilder.Build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);

    // Allocate a descriptor set for the draw image
    m_DrawImageDescriptorSet = m_GlobalDescriptorAllocator.Allocate(m_Device, m_DrawImageDescriptorLayout);

    DescriptorWriter writer;
    writer.WriteImage(0, m_DrawImage.m_ImageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(m_Device, m_DrawImageDescriptorSet);

    m_MainDeletionQueue.PushFunction([&]() {
        m_GlobalDescriptorAllocator.DestroyPools(m_Device);
        vkDestroyDescriptorSetLayout(m_Device, m_DrawImageDescriptorLayout, nullptr);
    });

    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frameSizes =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
        };

        m_Frames[i].m_FrameDescriptors = DescriptorAllocatorGrowable{};
        constexpr uint32_t initialSets = 1000;
        m_Frames[i].m_FrameDescriptors.Init(m_Device, initialSets, frameSizes);

        m_MainDeletionQueue.PushFunction([&, i]() {
            m_Frames[i].m_FrameDescriptors.DestroyPools(m_Device);
        });
    }

    DescriptorLayoutBuilder gpuSceneDataDescriptorLayoutBuilder;
    gpuSceneDataDescriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    m_GPUSceneDataDescriptorLayout = gpuSceneDataDescriptorLayoutBuilder.Build(m_Device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    m_MainDeletionQueue.PushFunction([&]() {
        vkDestroyDescriptorSetLayout(m_Device, m_GPUSceneDataDescriptorLayout, nullptr);
    });

    DescriptorLayoutBuilder singleImageDescriptorLayoutBuilder;
    singleImageDescriptorLayoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    m_SingleImageDescriptorLayout = singleImageDescriptorLayoutBuilder.Build(m_Device, VK_SHADER_STAGE_FRAGMENT_BIT);

    m_MainDeletionQueue.PushFunction([&]() {
        vkDestroyDescriptorSetLayout(m_Device, m_SingleImageDescriptorLayout, nullptr);
    });
}

void VulkanRenderer::InitPipelines()
{
    InitBackgroundPipelines();

    InitRenderPipeline();
    InitPointPipeline();
}

void VulkanRenderer::InitImgui()
{
    constexpr uint32_t maxSets = 1000;
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, maxSets },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSets },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxSets },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxSets },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, maxSets },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, maxSets },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSets },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxSets },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, maxSets },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, maxSets },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, maxSets } };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;

    VkDescriptorPool imguiPool;
    VULKAN_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &imguiPool));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForVulkan(m_Window);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_Instance;
    initInfo.PhysicalDevice = m_ChosenGPU;
    initInfo.Device = m_Device;
    initInfo.QueueFamily = m_GraphicsQueueIndex;
    initInfo.Queue = m_GraphicsQueue;
    //initInfo.PipelineCache = YOUR_PIPELINE_CACHE;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = FRAME_OVERLAP;
    initInfo.ImageCount = FRAME_OVERLAP;
    //initInfo.Allocator = YOUR_ALLOCATOR;
    //initInfo.PipelineInfoMain.RenderPass = wd->RenderPass;
    //initInfo.PipelineInfoMain.Subpass = 0;
    initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    //initInfo.CheckVkResultFn = check_vk_result;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &m_SwapchainImageFormat;
    ImGui_ImplVulkan_Init(&initInfo);

    m_MainDeletionQueue.PushFunction([=]() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(m_Device, imguiPool, nullptr);
    });
}

void VulkanRenderer::InitDefaultData()
{
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

    std::vector<Vertex> vertices;
    vertices.resize(4);

    vertices[0].m_Position = { 0.7, -0.5, 0 };
    vertices[1].m_Position = { 0.5, 0.3, 0 };
    vertices[2].m_Position = { -0.5, -0.5, 0 };
    vertices[3].m_Position = { -0.5, 0.5, 0 };

    vertices[0].m_Color = { 0, 0, 0, 1 };
    vertices[1].m_Color = { 0.5, 0.5, 0.5, 1 };
    vertices[2].m_Color = { 1, 0, 0, 1 };
    vertices[3].m_Color = { 0, 1, 0, 1 };

    std::vector<uint32_t> indices;
    indices.resize(6);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 1;
    indices[5] = 3;

    mesh->GetVertices() = std::move(vertices);
    mesh->GetIndices() = std::move(indices);
    
    AllocateMeshBuffers<Vertex, uint32_t>(mesh->GetVertices(), mesh->GetIndices(), mesh->GetVertexBuffer(), mesh->GetIndexBuffer(), mesh->GetVertexBufferAddress());

    m_DrawContext.m_DrawMeshes.push_back(mesh);

    m_MainDeletionQueue.PushFunction([=]() {
        DestroyBuffer(mesh->GetVertexBuffer());
        DestroyBuffer(mesh->GetIndexBuffer());
        m_DrawContext.m_DrawMeshes.clear();
    });

    std::shared_ptr<RenderPoint> point1 = std::make_shared<RenderPoint>();
    std::shared_ptr<RenderPoint> point2 = std::make_shared<RenderPoint>();
    std::shared_ptr<RenderPoint> point3 = std::make_shared<RenderPoint>();
    std::shared_ptr<RenderPoint> point4 = std::make_shared<RenderPoint>();

    point1->GetVertex().m_Position = { 0.5, 0.5, 0 };
    point2->GetVertex().m_Position = { -0.5, 0.5, 0 };
    point3->GetVertex().m_Position = { 0.5, -0.5, 0 };
    point4->GetVertex().m_Position = { -0.5, -0.5, 0 };

    point1->GetVertex().m_Color = { 0, 0, 0, 1 };
    point2->GetVertex().m_Color = { 0.5, 0.5, 0.5, 1 };
    point3->GetVertex().m_Color = { 1, 0, 0, 1 };
    point4->GetVertex().m_Color = { 0, 1, 0, 1 };

    AllocatePointBuffers<Vertex>(std::span<Vertex>{ &point1->GetVertex(), 1 }, point1->GetVertexBuffer(), point1->GetVertexBufferAddress());
    AllocatePointBuffers<Vertex>(std::span<Vertex>{ &point2->GetVertex(), 1 }, point2->GetVertexBuffer(), point2->GetVertexBufferAddress());
    AllocatePointBuffers<Vertex>(std::span<Vertex>{ &point3->GetVertex(), 1 }, point3->GetVertexBuffer(), point3->GetVertexBufferAddress());
    AllocatePointBuffers<Vertex>(std::span<Vertex>{ &point4->GetVertex(), 1 }, point4->GetVertexBuffer(), point4->GetVertexBufferAddress());

    m_DrawContext.m_DrawPoints.push_back(point1);
    m_DrawContext.m_DrawPoints.push_back(point2);
    m_DrawContext.m_DrawPoints.push_back(point3);
    m_DrawContext.m_DrawPoints.push_back(point4);

    m_MainDeletionQueue.PushFunction([=]() {
        DestroyBuffer(point1->GetVertexBuffer());
        DestroyBuffer(point2->GetVertexBuffer());
        DestroyBuffer(point3->GetVertexBuffer());
        DestroyBuffer(point4->GetVertexBuffer());
        m_DrawContext.m_DrawPoints.clear();
    });
}

void VulkanRenderer::CreateSwapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder{ m_ChosenGPU, m_Device, m_Surface };
    m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    VkSurfaceFormatKHR surfaceFormat;
    surfaceFormat.format = m_SwapchainImageFormat;
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(surfaceFormat)
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build().value();

    m_SwapchainExtent = vkbSwapchain.extent;
    m_Swapchain = vkbSwapchain.swapchain;
    m_SwapchainImages = vkbSwapchain.get_images().value();
    m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanRenderer::DestroySwapchain()
{
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    for (size_t i = 0; i < m_SwapchainImageViews.size(); ++i)
    {
        vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
    }
}

void VulkanRenderer::ResizeSwapchain()
{
    vkDeviceWaitIdle(m_Device);

    DestroySwapchain();

    int width, height;
    SDL_GetWindowSize(m_Window, &width, &height);
    m_WindowExtent.width = width;
    m_WindowExtent.height = height;

    CreateSwapchain(m_WindowExtent.width, m_WindowExtent.height);
    m_ResizeRequested = false;
}

AllocatedBuffer VulkanRenderer::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    AllocatedBuffer newBuffer;
    m_MemoryManager.AllocateBuffer(&bufferInfo, &vmaAllocInfo, &newBuffer.m_Buffer, &newBuffer.m_Allocation, &newBuffer.m_AllocationInfo);
    return newBuffer;
}

void VulkanRenderer::DestroyBuffer(const AllocatedBuffer& buffer)
{
    m_MemoryManager.DestroyBuffer(buffer.m_Buffer, buffer.m_Allocation);
}

template <typename V>
void VulkanRenderer::AllocatePointBuffers(const std::span<V>& vertices, AllocatedBuffer& vertexBuffer, VkDeviceAddress& vertexBufferAddress)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(V);

    VkBufferUsageFlags vertexBufferFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vertexBuffer = CreateBuffer(vertexBufferSize, vertexBufferFlags, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = vertexBuffer.m_Buffer;
    vertexBufferAddress = vkGetBufferDeviceAddress(m_Device, &deviceAddressInfo);

    UpdatePointBuffers<V>(vertices, vertexBuffer, vertexBufferAddress);
}

template <typename V, typename I>
void VulkanRenderer::AllocateMeshBuffers(const std::span<V>& vertices, const std::span<I>& indices, AllocatedBuffer& vertexBuffer, AllocatedBuffer& indexBuffer, VkDeviceAddress& vertexBufferAddress)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(V);
    const size_t indexBufferSize = indices.size() * sizeof(I);

    VkBufferUsageFlags vertexBufferFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vertexBuffer = CreateBuffer(vertexBufferSize, vertexBufferFlags, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferUsageFlags indexBufferFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    indexBuffer = CreateBuffer(indexBufferSize, indexBufferFlags, VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAddressInfo.buffer = vertexBuffer.m_Buffer;
    vertexBufferAddress = vkGetBufferDeviceAddress(m_Device, &deviceAddressInfo);

    UpdateMeshBuffers<V, I>(vertices, indices, vertexBuffer, indexBuffer, vertexBufferAddress);
}

template <typename V>
void VulkanRenderer::UpdatePointBuffers(const std::span<V>& vertices, AllocatedBuffer& vertexBuffer, VkDeviceAddress& vertexBufferAddress)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(V);

    AllocatedBuffer stagingBuffer = CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    void* data = stagingBuffer.m_Allocation->GetMappedData();
    memcpy(data, vertices.data(), vertexBufferSize);
    ImmediateSubmit([&](VkCommandBuffer commandBuffer) {
        VkBufferCopy vertexCopy{ 0 };
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.m_Buffer, vertexBuffer.m_Buffer, 1, &vertexCopy);
    });

    DestroyBuffer(stagingBuffer);
}

template <typename V, typename I>
void VulkanRenderer::UpdateMeshBuffers(const std::span<V>& vertices, const std::span<I>& indices, AllocatedBuffer& vertexBuffer, AllocatedBuffer& indexBuffer, VkDeviceAddress& vertexBufferAddress)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(V);
    const size_t indexBufferSize = indices.size() * sizeof(I);

    AllocatedBuffer stagingBuffer = CreateBuffer(vertexBufferSize + indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    void* data = stagingBuffer.m_Allocation->GetMappedData();
    memcpy(data, vertices.data(), vertexBufferSize);
    memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);
    ImmediateSubmit([&](VkCommandBuffer commandBuffer) {
        VkBufferCopy vertexCopy{ 0 };
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.m_Buffer, vertexBuffer.m_Buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy{ 0 };
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;
        vkCmdCopyBuffer(commandBuffer, stagingBuffer.m_Buffer, indexBuffer.m_Buffer, 1, &indexCopy);
    });

    DestroyBuffer(stagingBuffer);
}

void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer commandBuffer)>&& function)
{
    VULKAN_CHECK(vkResetFences(m_Device, 1, &m_ImmediateFence));
    VULKAN_CHECK(vkResetCommandBuffer(m_ImmediateCommandBuffer, 0));

    VkCommandBuffer commandBuffer = m_ImmediateCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = VulkanUtils::Command::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VULKAN_CHECK(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));
    function(commandBuffer);
    VULKAN_CHECK(vkEndCommandBuffer(commandBuffer));

    VkCommandBufferSubmitInfo cmdInfo = VulkanUtils::Command::CommandBufferSubmitInfo(commandBuffer);
    VkSubmitInfo2 submit = VulkanUtils::Sync::SubmitInfo(&cmdInfo, nullptr, nullptr);

    VULKAN_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, m_ImmediateFence));
    VULKAN_CHECK(vkWaitForFences(m_Device, 1, &m_ImmediateFence, true, UINT64_MAX));
}

void VulkanRenderer::InitBackgroundPipelines()
{
    VkPipelineLayoutCreateInfo computeLayout{};
    computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computeLayout.pNext = nullptr;
    computeLayout.pSetLayouts = &m_DrawImageDescriptorLayout;
    computeLayout.setLayoutCount = 1;

    VkPushConstantRange pushConstant{};
    pushConstant.offset = 0;
    pushConstant.size = sizeof(ComputePushConstants);
    pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    computeLayout.pPushConstantRanges = &pushConstant;
    computeLayout.pushConstantRangeCount = 1;

    VULKAN_CHECK(vkCreatePipelineLayout(m_Device, &computeLayout, nullptr, &m_BackgroundPipelineLayout));

    Shader backgroundShader{ m_Device, "shaders/background.comp.spv" };

    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.pNext = nullptr;
    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.module = backgroundShader.GetShader();
    stageInfo.pName = "main";

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.layout = m_BackgroundPipelineLayout;
    computePipelineCreateInfo.stage = stageInfo;

    m_BackgroundData.m_Data1 = glm::vec4{ 0, 0, 0, 1 };
    m_BackgroundData.m_Data2 = glm::vec4{ 0, 0, 0, 1 };

    VULKAN_CHECK(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_BackgroundPipeline));

    m_MainDeletionQueue.PushFunction([&]()
    {
        vkDestroyPipelineLayout(m_Device, m_BackgroundPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_BackgroundPipeline, nullptr);
    });
}

void VulkanRenderer::InitRenderPipeline()
{
    m_MeshRenderer.InitPipeline(m_Device, m_DrawImage.m_ImageFormat, m_DepthImage.m_ImageFormat, std::vector<VkDescriptorSetLayout>{ m_SingleImageDescriptorLayout });

    m_MainDeletionQueue.PushFunction([&]() {
        m_MeshRenderer.DeletePipeline(m_Device);
    });
}

void VulkanRenderer::InitPointPipeline()
{
    m_PointRenderer.InitPipeline(m_Device, m_DrawImage.m_ImageFormat, m_DepthImage.m_ImageFormat);

    m_MainDeletionQueue.PushFunction([&]() {
        m_PointRenderer.DeletePipeline(m_Device);
    });
}

void VulkanRenderer::Draw()
{
    UpdateScene();

    constexpr uint64_t oneSecondInNanoseconds = 1000000000;
    FrameData& currentFrame = GetCurrentFrame();

    // Wait for the previous frame to finish rendering
    VULKAN_CHECK(vkWaitForFences(m_Device, 1, &currentFrame.m_RenderFence, true, oneSecondInNanoseconds));
    currentFrame.m_FrameDeletionQueue.Flush();
    //currentFrame.m_FrameDescriptors.ClearPools(m_Device);
    VULKAN_CHECK(vkResetFences(m_Device, 1, &currentFrame.m_RenderFence));

    // Acquire next image from the swapchain
    uint32_t swapchainImageIndex;
    VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, oneSecondInNanoseconds, currentFrame.m_SwapchainSemaphore, nullptr, &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_ResizeRequested = true;
        return;
    }
    else
    {
        VULKAN_CHECK(result);
    }

    VkCommandBuffer commandBuffer = currentFrame.m_CommandBuffer;
    VULKAN_CHECK(vkResetCommandBuffer(commandBuffer, 0));

    m_DrawExtent.width = std::min(m_SwapchainExtent.width, m_DrawImage.m_ImageExtent.width) * (uint32_t)m_RenderScale;
    m_DrawExtent.height = std::min(m_SwapchainExtent.height, m_DrawImage.m_ImageExtent.height) * (uint32_t)m_RenderScale;

    // Begin command buffer recording; we will use this buffer just once, so we specify it in the flags
    VkCommandBufferBeginInfo commandBufferBeginInfo = VulkanUtils::Command::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VULKAN_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

    // Transition our main draw image into general layout so we can write into it
    // We will overwrite everything so we don't need to know the old layout
    VulkanUtils::Image::TransitionImage(commandBuffer, m_DrawImage.m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    DrawBackground(commandBuffer);

    VulkanUtils::Image::TransitionImage(commandBuffer, m_DrawImage.m_Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VulkanUtils::Image::TransitionImage(commandBuffer, m_DepthImage.m_Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    DrawGeometry(commandBuffer);

    // Make both our draw image and the swap chain image transition to the correct transfer layouts
    VkImage& swapchainImage = m_SwapchainImages[swapchainImageIndex];
    VkImageView& swapchainImageView = m_SwapchainImageViews[swapchainImageIndex];
    VulkanUtils::Image::TransitionImage(commandBuffer, m_DrawImage.m_Image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VulkanUtils::Image::TransitionImage(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy the draw image into the swapchain
    VulkanUtils::Image::CopyImageToImage(commandBuffer, m_DrawImage.m_Image, swapchainImage, m_DrawExtent, m_SwapchainExtent);

    // Make the swapchain image transition to the presentation layout
    VulkanUtils::Image::TransitionImage(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    DrawImGui(commandBuffer, swapchainImageView);

    VulkanUtils::Image::TransitionImage(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VULKAN_CHECK(vkEndCommandBuffer(commandBuffer));

    // Prepare submission of command buffer to queue
    VkCommandBufferSubmitInfo commandBufferSubmitInfo = VulkanUtils::Command::CommandBufferSubmitInfo(commandBuffer);

    // We will wait on the presentation semaphore, since that semaphore is signaled when the swapchain is ready
    // We will signal the render semaphore to signal that rendering has finished
    VkSemaphoreSubmitInfo waitInfo = VulkanUtils::Sync::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.m_SwapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = VulkanUtils::Sync::SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, currentFrame.m_RenderSemaphore);

    // Submit the commands to graphics queue for rendering
    VkSubmitInfo2 submit = VulkanUtils::Sync::SubmitInfo(&commandBufferSubmitInfo, &signalInfo, &waitInfo);
    VULKAN_CHECK(vkQueueSubmit2(m_GraphicsQueue, 1, &submit, currentFrame.m_RenderFence));

    // Present the image drawn into the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &m_Swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &currentFrame.m_RenderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    VkResult presentResult = vkQueuePresentKHR(m_GraphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_ResizeRequested = true;
    }
    else
    {
        VULKAN_CHECK(presentResult);
    }

    ++m_FrameNumber;
}

void VulkanRenderer::DrawBackground(VkCommandBuffer commandBuffer)
{
    // Bind the gradient drawing compute pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_BackgroundPipeline);

    // Bind the descriptor set containing the draw image for the compute pipeline
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_BackgroundPipelineLayout, 0, 1, &m_DrawImageDescriptorSet, 0, nullptr);

    vkCmdPushConstants(commandBuffer, m_BackgroundPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &m_BackgroundData);

    // Execute the compute pipeline dispatch; we are using 16x16 workgroup size so we need to divide by it
    static constexpr uint32_t workgroupSize = 16;
    vkCmdDispatch(commandBuffer, std::ceil(m_DrawExtent.width / workgroupSize), std::ceil(m_DrawExtent.height / workgroupSize), 1);
}

void VulkanRenderer::DrawGeometry(VkCommandBuffer commandBuffer)
{
    FrameData& currentFrame = GetCurrentFrame();
    AllocatedBuffer gpuSceneDataBuffer = CreateBuffer(sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    currentFrame.m_FrameDeletionQueue.PushFunction([=, this]() {
        DestroyBuffer(gpuSceneDataBuffer);
    });

    GPUSceneData* sceneUniformData = (GPUSceneData*)gpuSceneDataBuffer.m_Allocation->GetMappedData();
    *sceneUniformData = m_SceneData;

    VkDescriptorSet gpuSceneDescriptorSet = currentFrame.m_FrameDescriptors.Allocate(m_Device, m_GPUSceneDataDescriptorLayout);
    DescriptorWriter gpuSceneDescriptorWriter;
    gpuSceneDescriptorWriter.WriteBuffer(0, gpuSceneDataBuffer.m_Buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    gpuSceneDescriptorWriter.UpdateSet(m_Device, gpuSceneDescriptorSet);

    VkRenderingAttachmentInfo colorAttachment = VulkanUtils::Render::RenderingAttachmentInfo(m_DrawImage.m_ImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingAttachmentInfo depthAttachment = VulkanUtils::Render::RenderingDepthAttachmentInfo(m_DepthImage.m_ImageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = VulkanUtils::Render::RenderingInfo(m_DrawExtent, &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(commandBuffer, &renderInfo);
    
    m_MeshRenderer.Draw(commandBuffer, m_DrawExtent, m_DrawContext.m_DrawMeshes, std::vector{ gpuSceneDescriptorSet });
    m_PointRenderer.Draw(commandBuffer, m_DrawExtent, m_DrawContext.m_DrawPoints);

    vkCmdEndRendering(commandBuffer);
}

void VulkanRenderer::DrawImGui(VkCommandBuffer commandBuffer, VkImageView targetImageView)
{
    VkRenderingAttachmentInfo colorAttachment = VulkanUtils::Render::RenderingAttachmentInfo(targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = VulkanUtils::Render::RenderingInfo(m_SwapchainExtent, &colorAttachment, nullptr);

    vkCmdBeginRendering(commandBuffer, &renderInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRendering(commandBuffer);
}

void VulkanRenderer::UpdateScene()
{
    //for (std::shared_ptr<Mesh> mesh : m_DrawContext.m_DrawMeshes)
    //{
    //    mesh->Randomize();
    //    UpdateMeshBuffers<Vertex, uint32_t>(mesh->GetVertices(), mesh->GetIndices(), mesh->GetVertexBuffer(), mesh->GetIndexBuffer(), mesh->GetVertexBufferAddress());
    //}
}