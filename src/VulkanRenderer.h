#pragma once

#include <array>
#include <deque>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vk_mem_alloc.h>
#include <VulkanDescriptor.h>
#include <vulkan/vulkan.h>

struct SDL_Window;

struct AllocatedImage
{
	VkImage m_Image;
	VkImageView m_ImageView;
	VmaAllocation m_Allocation;
	VkExtent3D m_ImageExtent;
	VkFormat m_ImageFormat;
};

struct AllocatedBuffer
{

};

struct ComputePushConstants
{
	glm::vec4 m_Data1;
	glm::vec4 m_Data2;
	glm::vec4 m_Data3;
	glm::vec4 m_Data4;
};

struct DeletionQueue
{
	std::deque<std::function<void()>> m_Deletors;

	void PushFunction(std::function<void()>&& function)
	{
		m_Deletors.push_back(function);
	}

	void Flush()
	{
		for (auto it = m_Deletors.rbegin(); it < m_Deletors.rend(); ++it)
		{
			(*it)();
		}

		m_Deletors.clear();
	}
};

struct FrameData
{
	VkCommandPool m_CommandPool;
	VkCommandBuffer m_CommandBuffer;

	VkSemaphore m_SwapchainSemaphore;
	VkSemaphore m_RenderSemaphore;
	VkFence m_RenderFence;

	DeletionQueue m_FrameDeletionQueue;
	//DescriptorAllocatorGrowable m_FrameDescriptors;
};

class VulkanRenderer
{
public:
	void Init();
	void Run();
	void Cleanup();

private:
	void InitVulkan();
	void InitSwapchain();
	void InitCommands();
	void InitSyncStructures();
	void InitDescriptors();
	void InitPipelines();

	void CreateSwapchain(uint32_t width, uint32_t height);
	void DestroySwapchain();
	void ResizeSwapchain();

	void InitBackgroundPipelines();

	void Draw();

	void DrawBackground(VkCommandBuffer commandBuffer);

	FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; };

private:
	static constexpr uint32_t WINDOW_WIDTH = 1200;
	static constexpr uint32_t WINDOW_HEIGHT = 800;
	static constexpr uint32_t FRAME_OVERLAP = 2;

	SDL_Window* m_Window = nullptr;
	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkSurfaceKHR m_Surface;
	VkDevice m_Device;
	VkPhysicalDevice m_ChosenGPU;
	VkExtent2D m_WindowExtent{ WINDOW_WIDTH, WINDOW_HEIGHT };

	VkSwapchainKHR m_Swapchain;
	VkFormat m_SwapchainImageFormat;
	VkExtent2D m_SwapchainExtent;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;

	std::array<FrameData, FRAME_OVERLAP> m_Frames;

	AllocatedImage m_DrawImage;
	VkDescriptorSet m_DrawImageDescriptorSet;
	VkDescriptorSetLayout m_DrawImageDescriptorLayout;

	VmaAllocator m_Allocator;
	DescriptorAllocatorGrowable m_GlobalDescriptorAllocator;

	VkQueue m_GraphicsQueue;
	uint32_t m_GraphicsQueueIndex = UINT32_MAX;

	VkPipeline m_BackgroundPipeline;
	VkPipelineLayout m_BackgroundPipelineLayout;
	ComputePushConstants m_BackgroundData{};

	VkExtent2D m_DrawExtent;
	float m_RenderScale = 1.f;

	DeletionQueue m_MainDeletionQueue;

	bool m_IsInitialized{ false };
	bool m_ResizeRequested = false;

	uint8_t m_FrameNumber{ 0 };
};