#pragma once

#include <array>
#include <deque>
#include <engine/FrameData.h>
#include <functional>
#include <memory>
#include <mutex>
#include <renderer/MeshRenderer.h>
#include <renderer/PointRenderer.h>
#include <renderer/VulkanDescriptor.h>
#include <renderer/VulkanMemoryManager.h>
#include <renderer/VulkanTypes.h>
#include <SDL3/SDL.h>
#include <tracy/Tracy.hpp>
#include <vector>
#include <vk_mem_alloc.h>
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

struct ThreadData
{
	VkCommandBuffer m_ImmediateCommandBuffer;
	VkCommandPool m_ImmediateCommandPool;
};

struct VulkanFrameData
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
	std::vector<std::shared_ptr<RenderPoint>> m_EntityDrawPoints;
	std::vector<std::shared_ptr<Mesh>> m_EntityDrawMeshes;

	std::unordered_map<EntityID, std::shared_ptr<RenderPoint>> m_EnginePoints;
	std::unordered_map<EntityID, std::shared_ptr<Mesh>> m_EngineMeshes;

	TracyLockable(std::mutex, m_EntityDrawPointsMutex);
	TracyLockable(std::mutex, m_EntityDrawMeshesMutex);
	TracyLockable(std::mutex, m_EnginePointsMutex);
	TracyLockable(std::mutex, m_EngineMeshesMutex);
};

class VulkanRenderer
{
public:
	void Init(size_t threadCount);
	void Run(FrameData& frameData);
	void ProcessSDLEvent(SDL_Event& e);
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

	void AllocatePointBuffers(RenderPoint& point);
	void AllocateMeshBuffers(Mesh& mesh);

	void UpdatePointBuffers(RenderPoint& point, ThreadData& threadData);
	void UpdateMeshBuffers(Mesh& mesh, ThreadData& threadData);

	void BeginTransferCommandBuffer(VkCommandBuffer commandBuffer);
	void EndTransferCommandBuffer(VkCommandBuffer commandBuffer);
	void QueueTransferCommand(VkCommandBuffer commandBuffer, std::function<void(VkCommandBuffer commandBuffer)>&& function);
	void SubmitTransferQueue(VkCommandBuffer commandBuffer);

	void InitBackgroundPipelines();
	void InitRenderPipeline();
	void InitPointPipeline();

	void Draw(FrameData& frameData);

	void DrawBackground(VkCommandBuffer commandBuffer);
	void DrawGeometry(VkCommandBuffer commandBuffer);
	void DrawImGui(VkCommandBuffer commandBuffer, VkImageView targetImageView);

	void UpdateScene(FrameData& frameData);

	VulkanFrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; };

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

	std::array<VulkanFrameData, FRAME_OVERLAP> m_Frames;

	std::vector<ThreadData> m_ThreadData;

	AllocatedImage m_DrawImage;
	VkDescriptorSet m_DrawImageDescriptorSet;
	VkDescriptorSetLayout m_DrawImageDescriptorLayout;

	AllocatedImage m_DepthImage;

	GPUSceneData m_SceneData;
	VkDescriptorSetLayout m_GPUSceneDataDescriptorLayout;

	VkDescriptorSetLayout m_SingleImageDescriptorLayout;

	VulkanMemoryManager m_MemoryManager;
	DescriptorAllocatorGrowable m_GlobalDescriptorAllocator;

	VkQueue m_TransferQueue;
	uint32_t m_TransferQueueIndex = UINT32_MAX;
	VkFence m_TransferFence;

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