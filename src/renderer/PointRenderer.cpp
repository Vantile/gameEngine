#include <renderer/PointRenderer.h>

#include <renderer/PipelineBuilder.h>
#include <renderer/RenderPoint.h>
#include <renderer/Shader.h>
#include <renderer/VulkanUtils.h>
#include <tracy/Tracy.hpp>

void PointRenderer::InitPipeline(VkDevice device, VkFormat drawImageFormat, VkFormat depthImageFormat)
{
    Shader vertexShader(device, "shaders/point.vert.spv");
    Shader fragmentShader(device, "shaders/point.frag.spv");

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(PushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanUtils::Pipeline::PipelineLayoutCreateInfo();
    pipelineLayoutCreateInfo.pPushConstantRanges = &bufferRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.setLayoutCount = 0;

    VULKAN_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PointPipelineLayout));

    PipelineBuilder builder;
    builder.m_PipelineLayout = m_PointPipelineLayout;
    builder.SetShaders(vertexShader.GetShader(), fragmentShader.GetShader());
    builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    builder.SetPolygonMode(VK_POLYGON_MODE_POINT);
    builder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    builder.SetMultisamplingNone();
    builder.DisableBlending();
    builder.DisableDepthtest();

    builder.SetColorAttachmentFormat(drawImageFormat);
    builder.SetDepthFormat(depthImageFormat);

    m_PointPipeline = builder.BuildPipeline(device);
}

void PointRenderer::DeletePipeline(VkDevice device)
{
    vkDestroyPipelineLayout(device, m_PointPipelineLayout, nullptr);
    vkDestroyPipeline(device, m_PointPipeline, nullptr);
}

void PointRenderer::Draw(VkCommandBuffer commandBuffer, VkExtent2D drawExtent, const std::vector<std::shared_ptr<RenderPoint>>& points)
{
    ZoneScoped;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PointPipeline);

    uint32_t width = drawExtent.width;
    uint32_t height = drawExtent.height;

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = width;
    scissor.extent.height = height;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (std::shared_ptr<RenderPoint> point : points)
    {
        ZoneScopedN("PointRenderer::Draw PointDraw");

        PushConstants pushConstants;
        pushConstants.m_Vertex = point->GetVertex();
        vkCmdPushConstants(commandBuffer, m_PointPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);

        vkCmdDraw(commandBuffer, 1, 1, 0, 0);
    }
}