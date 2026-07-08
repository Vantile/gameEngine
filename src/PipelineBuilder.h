#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class PipelineBuilder
{
public:
	PipelineBuilder() { Clear(); }
	void Clear();
	VkPipeline BuildPipeline(VkDevice device);
	void SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
	void SetInputTopology(VkPrimitiveTopology topology);
	void SetPolygonMode(VkPolygonMode mode);
	void SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
	void SetMultisamplingNone();

	void DisableBlending();
	void EnableBlendingAdditive();
	void EnableBlendingAlphablend();

	void SetColorAttachmentFormat(VkFormat format);
	void SetDepthFormat(VkFormat format);
	void EnableDepthtest(bool depthWriteEnable, VkCompareOp op);
	void DisableDepthtest();

public:
	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

	VkPipelineInputAssemblyStateCreateInfo m_InputAssembly;
	VkPipelineRasterizationStateCreateInfo m_Rasterizer;
	VkPipelineColorBlendAttachmentState m_ColorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo m_Multisampling;
	VkPipelineLayout m_PipelineLayout;
	VkPipelineDepthStencilStateCreateInfo m_DepthStencil;
	VkPipelineRenderingCreateInfo m_RenderInfo;
	VkFormat m_ColorAttachmentFormat;
};