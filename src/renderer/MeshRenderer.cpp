#include <renderer/MeshRenderer.h>

#include <renderer/Mesh.h>
#include <renderer/PipelineBuilder.h>
#include <renderer/Shader.h>
#include <renderer/VulkanUtils.h>
#include <tracy/Tracy.hpp>

void MeshRenderer::InitPipeline(VkDevice device, VkFormat drawImageFormat, VkFormat depthImageFormat, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
    Shader vertexShader(device, "shaders/colored_triangle.vert.spv");
    Shader fragmentShader(device, "shaders/colored_triangle.frag.spv");

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(PushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = VulkanUtils::Pipeline::PipelineLayoutCreateInfo();
    pipelineLayoutCreateInfo.pPushConstantRanges = &bufferRange;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();

    VULKAN_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_MeshPipelineLayout));

    PipelineBuilder builder;
    builder.m_PipelineLayout = m_MeshPipelineLayout;
    builder.SetShaders(vertexShader.GetShader(), fragmentShader.GetShader());
    builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
    builder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    builder.SetMultisamplingNone();
    builder.DisableBlending();
    //builder.EnableBlendingAdditive();
    builder.EnableDepthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

    builder.SetColorAttachmentFormat(drawImageFormat);
    builder.SetDepthFormat(depthImageFormat);

    m_MeshPipeline = builder.BuildPipeline(device);
}

void MeshRenderer::DeletePipeline(VkDevice device)
{
    vkDestroyPipelineLayout(device, m_MeshPipelineLayout, nullptr);
    vkDestroyPipeline(device, m_MeshPipeline, nullptr);
}

void MeshRenderer::Draw(VkCommandBuffer commandBuffer, VkExtent2D drawExtent, const std::vector<Mesh*>& meshes, const std::vector<VkDescriptorSet>& descriptorSets)
{
    ZoneScoped;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipeline);

    // TODO: This assumes that there is one set per layout; I need to create a class to better encapsulate descriptor sets and layouts
    for (size_t i = 0; i < descriptorSets.size(); ++i)
    {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
    }

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

    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;
    for (Mesh* mesh : meshes)
    {
        ZoneScopedN("MeshRenderer::Draw MeshDraw")

        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MeshPipelineLayout, 1, 1, &draw.m_Material->m_MaterialSet, 0, nullptr);

        if (mesh->GetIndexBuffer().m_Buffer != lastIndexBuffer)
        {
            lastIndexBuffer = mesh->GetIndexBuffer().m_Buffer;
            vkCmdBindIndexBuffer(commandBuffer, mesh->GetIndexBuffer().m_Buffer, 0, VK_INDEX_TYPE_UINT32);
        }

        PushConstants pushConstants;
        pushConstants.m_VertexBuffer = mesh->GetVertexBufferAddress();
        pushConstants.m_WorldMatrix = glm::mat4{ 1.f };
        vkCmdPushConstants(commandBuffer, m_MeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &pushConstants);

        vkCmdDrawIndexed(commandBuffer, mesh->GetIndices().size(), 1, 0, 0, 0);
    }
}