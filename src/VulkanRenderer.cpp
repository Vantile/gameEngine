#include <VulkanRenderer.h>

#include <assert.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <Shader.h>
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <VulkanUtils.h>

void VulkanRenderer::Run()
{
    SDL_Event e;
    bool bQuit = false;

    // main loop
    while (!bQuit)
    {
        //auto start = std::chrono::system_clock::now();
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            // close the window when user alt-f4s or clicks the X button
            if (e.type == SDL_EVENT_QUIT)
                bQuit = true;

            //if (e.type == SDL_WINDOWEVENT)
            //{
            //    if (e.window.event == SDL_WINDOWEVENT_MINIMIZED)
            //    {
            //        //m_StopRendering = true;
            //    }

            //    if (e.window.event == SDL_WINDOWEVENT_RESTORED)
            //    {
            //        //m_StopRendering = false;
            //    }
            //}

            //m_MainCamera.ProcessSDLEvent(e);
            //ImGui_ImplSDL2_ProcessEvent(&e);
        }

        // do not draw if we are minimized
        //if (m_StopRendering)
        //{
        //    // throttle the speed to avoid the endless spinning
        //    std::this_thread::sleep_for(std::chrono::milliseconds(100));
        //    continue;
        //}

        if (m_ResizeRequested)
        {
            ResizeSwapchain();
        }

        //ImGui_ImplVulkan_NewFrame();
        //ImGui_ImplSDL2_NewFrame();
        //ImGui::NewFrame();

        //if (ImGui::Begin("background"))
        //{
        //    ImGui::SliderFloat("Render Scale", &m_RenderScale, 0.3f, 1.f);

        //    ComputeEffect& selected = m_BackgroundEffects[m_CurrentBackgroundEffect];

        //    ImGui::Text("Selected effect: ", selected.m_Name);

        //    ImGui::SliderInt("Effect Index", &m_CurrentBackgroundEffect, 0, m_BackgroundEffects.size() - 1);

        //    ImGui::InputFloat4("data1", (float*)&selected.m_Data.m_Data1);
        //    ImGui::InputFloat4("data2", (float*)&selected.m_Data.m_Data2);
        //    ImGui::InputFloat4("data3", (float*)&selected.m_Data.m_Data3);
        //    ImGui::InputFloat4("data4", (float*)&selected.m_Data.m_Data4);
        //}
        //ImGui::End();

        //if (ImGui::Begin("Stats"))
        //{
        //    ImGui::Text("frametime %f ms", m_Stats.m_FrameTime);
        //    ImGui::Text("draw time %f ms", m_Stats.m_MeshDrawTime);
        //    ImGui::Text("update time %f ms", m_Stats.m_SceneUpdateTime);
        //    ImGui::Text("triangles %i", m_Stats.m_TriangleCount);
        //    ImGui::Text("draws %i", m_Stats.m_DrawCallCount);
        //    ImGui::Text("visible check %f ms", m_Stats.m_VisibleCheckTime);
        //}
        //ImGui::End();

        //ImGui::Render();

        Draw();

        //auto end = std::chrono::system_clock::now();
        //auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        //m_Stats.m_FrameTime = elapsed.count() / 1000.f;
    }
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
    //InitImgui();
    //InitDefaultData();
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

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = m_ChosenGPU;
    allocatorInfo.device = m_Device;
    allocatorInfo.instance = m_Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &m_Allocator);

    m_MainDeletionQueue.PushFunction([&]() {
        vmaDestroyAllocator(m_Allocator);
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

    vmaCreateImage(m_Allocator, &imageInfo, &imageAllocationInfo, &m_DrawImage.m_Image, &m_DrawImage.m_Allocation, nullptr);

    // Build an image-view for the draw image to use for rendering
    VkImageViewCreateInfo viewInfo = VulkanUtils::Image::ImageViewCreateInfo(m_DrawImage.m_ImageFormat, m_DrawImage.m_Image, VK_IMAGE_ASPECT_COLOR_BIT);
    VULKAN_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_DrawImage.m_ImageView));

    //m_DepthImage.m_ImageFormat = VK_FORMAT_D32_SFLOAT;
    //m_DepthImage.m_ImageExtent = drawImageExtent;
    //VkImageUsageFlags depthImageUsages{};
    //depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    //VkImageCreateInfo depthImageInfo = VulkanUtils::Image::ImageCreateInfo(m_DepthImage.m_ImageFormat, depthImageUsages, drawImageExtent);
    //vmaCreateImage(m_Allocator, &depthImageInfo, &imageAllocationInfo, &m_DepthImage.m_Image, &m_DepthImage.m_Allocation, nullptr);

    //VkImageViewCreateInfo depthViewInfo = VulkanUtils::Image::ImageViewCreateInfo(m_DepthImage.m_ImageFormat, m_DepthImage.m_Image, VK_IMAGE_ASPECT_DEPTH_BIT);
    //VULKAN_CHECK(vkCreateImageView(m_Device, &depthViewInfo, nullptr, &m_DepthImage.m_ImageView));

    m_MainDeletionQueue.PushFunction([=]() {
        vkDestroyImageView(m_Device, m_DrawImage.m_ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_DrawImage.m_Image, m_DrawImage.m_Allocation);

        //vkDestroyImageView(m_Device, m_DepthImage.m_ImageView, nullptr);
        //vmaDestroyImage(m_Allocator, m_DepthImage.m_Image, m_DepthImage.m_Allocation);
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

    //VULKAN_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_ImCommandPool));
    //VkCommandBufferAllocateInfo imAllocInfo = VulkanUtils::Command::CommandBufferAllocateInfo(m_ImCommandPool, 1);
    //VULKAN_CHECK(vkAllocateCommandBuffers(m_Device, &imAllocInfo, &m_ImCommandBuffer));
    //m_MainDeletionQueue.PushFunction([=]() {
    //    vkDestroyCommandPool(m_Device, m_ImCommandPool, nullptr);
    //});
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

    //VULKAN_CHECK(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_ImFence));
    //m_MainDeletionQueue.PushFunction([=]() {
    //    vkDestroyFence(m_Device, m_ImFence, nullptr);
    //});
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
}

void VulkanRenderer::InitPipelines()
{
    InitBackgroundPipelines();
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

    m_BackgroundData.m_Data1 = glm::vec4{ 1, 0, 0, 1 };
    m_BackgroundData.m_Data2 = glm::vec4{ 0, 0, 1, 1 };

    VULKAN_CHECK(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_BackgroundPipeline));

    m_MainDeletionQueue.PushFunction([&]()
    {
        vkDestroyPipelineLayout(m_Device, m_BackgroundPipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_BackgroundPipeline, nullptr);
    });
}

void VulkanRenderer::Draw()
{
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

    // Make both our draw image and the swap chain image transition to the correct transfer layouts
    VkImage& swapchainImage = m_SwapchainImages[swapchainImageIndex];
    VkImageView& swapchainImageView = m_SwapchainImageViews[swapchainImageIndex];
    VulkanUtils::Image::TransitionImage(commandBuffer, m_DrawImage.m_Image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VulkanUtils::Image::TransitionImage(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy the draw image into the swapchain
    VulkanUtils::Image::CopyImageToImage(commandBuffer, m_DrawImage.m_Image, swapchainImage, m_DrawExtent, m_SwapchainExtent);

    // Make the swapchain image transition to the presentation layout
    //VulkanUtils::Image::TransitionImage(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VulkanUtils::Image::TransitionImage(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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