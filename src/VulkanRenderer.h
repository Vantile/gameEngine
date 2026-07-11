#pragma once

#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <MeshRenderer.h>
#include <PointRenderer.h>
#include <vector>
#include <vk_mem_alloc.h>
#include <VulkanDescriptor.h>
#include <VulkanMemoryManager.h>
#include <VulkanTypes.h>
#include <vulkan/vulkan.h>

class Mesh;
class RenderPoint;
struct SDL_Window;

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
	DescriptorAllocatorGrowable m_FrameDescriptors;
};

struct WorldContext
{
	//std::vector<std::shared_ptr<Mesh>> m_DrawMeshes;
};

struct DrawContext
{
	std::vector<std::shared_ptr<RenderPoint>> m_DrawPoints;
	std::vector<std::shared_ptr<Mesh>> m_DrawMeshes;
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
	void InitImgui();
	void InitDefaultData();

	void CreateSwapchain(uint32_t width, uint32_t height);
	void DestroySwapchain();
	void ResizeSwapchain();

	AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void DestroyBuffer(const AllocatedBuffer& buffer);

	template <typename V>
	void AllocatePointBuffers(const std::span<V>& vertices, AllocatedBuffer& vertexBuffer, VkDeviceAddress& vertexBufferAddress);

	template <typename V, typename I>
	void AllocateMeshBuffers(const std::span<V>& vertices, const std::span<I>& indices, AllocatedBuffer& vertexBuffer, AllocatedBuffer& indexBuffer, VkDeviceAddress& vertexBufferAddress);

	template <typename V>
	void UpdatePointBuffers(const std::span<V>& vertices, AllocatedBuffer& vertexBuffer, VkDeviceAddress& vertexBufferAddress);

	template <typename V, typename I>
	void UpdateMeshBuffers(const std::span<V>& vertices, const std::span<I>& indices, AllocatedBuffer& vertexBuffer, AllocatedBuffer& indexBuffer, VkDeviceAddress& vertexBufferAddress);

	void ImmediateSubmit(std::function<void(VkCommandBuffer commandBuffer)>&& function);

	void InitBackgroundPipelines();
	void InitRenderPipeline();
	void InitPointPipeline();

	void Draw();

	void DrawBackground(VkCommandBuffer commandBuffer);
	void DrawGeometry(VkCommandBuffer commandBuffer);
	void DrawImGui(VkCommandBuffer commandBuffer, VkImageView targetImageView);

	void UpdateScene();

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

	VkFence m_ImmediateFence;
	VkCommandBuffer m_ImmediateCommandBuffer;
	VkCommandPool m_ImmediateCommandPool;

	AllocatedImage m_DrawImage;
	VkDescriptorSet m_DrawImageDescriptorSet;
	VkDescriptorSetLayout m_DrawImageDescriptorLayout;

	AllocatedImage m_DepthImage;

	GPUSceneData m_SceneData;
	VkDescriptorSetLayout m_GPUSceneDataDescriptorLayout;

	VkDescriptorSetLayout m_SingleImageDescriptorLayout;

	VulkanMemoryManager m_MemoryManager;
	DescriptorAllocatorGrowable m_GlobalDescriptorAllocator;

	VkQueue m_GraphicsQueue;
	uint32_t m_GraphicsQueueIndex = UINT32_MAX;

	VkPipeline m_BackgroundPipeline;
	VkPipelineLayout m_BackgroundPipelineLayout;
	ComputePushConstants m_BackgroundData{};

	PointRenderer m_PointRenderer;
	MeshRenderer m_MeshRenderer;

	DrawContext m_DrawContext;

	VkExtent2D m_DrawExtent;
	float m_RenderScale = 1.f;

	DeletionQueue m_MainDeletionQueue;

	bool m_IsInitialized{ false };
	bool m_ResizeRequested = false;

	uint8_t m_FrameNumber{ 0 };
};