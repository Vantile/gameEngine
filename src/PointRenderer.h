#pragma once

#include <memory>
#include <vector>
#include <Vertex.h>
#include <vulkan/vulkan.h>

class RenderPoint;

class PointRenderer
{
public:
	void InitPipeline(VkDevice device, VkFormat drawImageFormat, VkFormat depthImageFormat);
	void DeletePipeline(VkDevice device);

	void Draw(VkCommandBuffer commandBuffer, VkExtent2D drawExtent, const std::vector<std::shared_ptr<RenderPoint>>& points);

private:
	struct PushConstants
	{
		Vertex m_Vertex;
	};

	VkPipeline m_PointPipeline;
	VkPipelineLayout m_PointPipelineLayout;
};