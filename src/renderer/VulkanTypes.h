#pragma once

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

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
	VkBuffer m_Buffer;
	VmaAllocation m_Allocation;
	VmaAllocationInfo m_AllocationInfo;
};

struct ComputePushConstants
{
	glm::vec4 m_Data1;
	glm::vec4 m_Data2;
	glm::vec4 m_Data3;
	glm::vec4 m_Data4;
};

struct GPUMeshBuffers
{
	AllocatedBuffer m_IndexBuffer;
	AllocatedBuffer m_VertexBuffer;
	VkDeviceAddress m_VertexBufferAddress;
};

struct GPUSceneData
{
	glm::mat4 m_View;
	glm::mat4 m_Proj;
	glm::mat4 m_Viewproj;
	glm::vec4 m_AmbientColor;
	glm::vec4 m_SunlightDirection; // w for sun power
	glm::vec4 m_SunlightColor;
};