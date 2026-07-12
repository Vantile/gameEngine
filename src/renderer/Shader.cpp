#include <renderer/Shader.h>

#include <renderer/VulkanUtils.h>

Shader::Shader(VkDevice device, std::string_view filePath)
{
	m_Device = device;
	if (!VulkanUtils::Shader::LoadShaderModule(filePath.data(), device, &m_Shader))
	{
		std::printf("VULKAN: Error when building a shader.");
		assert(false);
	}
}

Shader::~Shader()
{
	vkDestroyShaderModule(m_Device, m_Shader, nullptr);
}