#pragma once

#include <glm/mat4x4.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class Mesh;

class MeshRenderer
{
public:
	void InitPipeline(VkDevice device, VkFormat drawImageFormat, VkFormat depthImageFormat, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	void DeletePipeline(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkExtent2D drawExtent, const std::vector<Mesh*>& points, const std::vector<VkDescriptorSet>& descriptorSets);

private:
	struct PushConstants
	{
		glm::mat4 m_WorldMatrix;
		VkDeviceAddress m_VertexBuffer;
	};

	VkPipeline m_MeshPipeline;
	VkPipelineLayout m_MeshPipelineLayout;
};