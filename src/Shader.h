#pragma once

#include <string_view>
#include <vulkan/vulkan.h>

class Shader
{
public:
	Shader(VkDevice device, std::string_view filePath);
	~Shader();
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	VkShaderModule& GetShader() { return m_Shader; }

private:
	VkDevice m_Device;
	VkShaderModule m_Shader;
};