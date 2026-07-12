#include <renderer/VulkanDescriptor.h>

#include <print>
#include <renderer/VulkanUtils.h>

void DescriptorAllocatorGrowable::Init(VkDevice device, uint32_t initialSets, std::span<PoolSizeRatio> poolRatios)
{
	m_Ratios.clear();

	for (PoolSizeRatio& ratio : poolRatios)
	{
		m_Ratios.push_back(ratio);
	}

	VkDescriptorPool newPool = CreatePool(device, initialSets, poolRatios);
	m_SetsPerPool = initialSets * 1.5;
	m_ReadyPools.push_back(newPool);
}

void DescriptorAllocatorGrowable::ClearPools(VkDevice device)
{
	for (VkDescriptorPool& pool : m_ReadyPools)
	{
		vkResetDescriptorPool(device, pool, 0);
	}

	for (VkDescriptorPool& pool : m_FullPools)
	{
		vkResetDescriptorPool(device, pool, 0);
		m_ReadyPools.push_back(pool);
	}

	m_FullPools.clear();
}

void DescriptorAllocatorGrowable::DestroyPools(VkDevice device)
{
	for (VkDescriptorPool& pool : m_ReadyPools)
	{
		vkDestroyDescriptorPool(device, pool, 0);
	}

	for (VkDescriptorPool& pool : m_FullPools)
	{
		vkDestroyDescriptorPool(device, pool, 0);
	}

	m_ReadyPools.clear();
	m_FullPools.clear();
}

VkDescriptorSet DescriptorAllocatorGrowable::Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext)
{
	VkDescriptorPool poolToUse = GetPool(device);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = pNext;
	allocInfo.descriptorPool = poolToUse;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
	{
		std::printf("VULKAN: Descriptor pool fragmented or out of memory; new pool created.");

		m_FullPools.push_back(poolToUse);
		poolToUse = GetPool(device);
		allocInfo.descriptorPool = poolToUse;

		VULKAN_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
	}
	else
	{
		VULKAN_CHECK(result);
	}

	m_ReadyPools.push_back(poolToUse);
	return descriptorSet;
}

VkDescriptorPool DescriptorAllocatorGrowable::GetPool(VkDevice device)
{
	VkDescriptorPool newPool;
	if (m_ReadyPools.size() != 0)
	{
		newPool = m_ReadyPools.back();
		m_ReadyPools.pop_back();
	}
	else
	{
		newPool = CreatePool(device, m_SetsPerPool, m_Ratios);
		m_SetsPerPool = m_SetsPerPool * 1.5;
		m_SetsPerPool = std::min(m_SetsPerPool, (uint32_t)4092);
	}

	return newPool;
}

VkDescriptorPool DescriptorAllocatorGrowable::CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for (const PoolSizeRatio& ratio : m_Ratios)
	{
		VkDescriptorPoolSize poolSize{
			.type = ratio.m_Type,
			.descriptorCount = uint32_t(ratio.m_Ratio * setCount)
		};
		poolSizes.push_back(poolSize);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = 0;
	poolInfo.maxSets = setCount;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	VkDescriptorPool newPool;
	vkCreateDescriptorPool(device, &poolInfo, nullptr, &newPool);
	return newPool;
}

void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding newBind{};
	newBind.binding = binding;
	newBind.descriptorCount = 1;
	newBind.descriptorType = type;
	m_Bindings.push_back(newBind);
}

void DescriptorLayoutBuilder::Clear()
{
	m_Bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
{
	for (VkDescriptorSetLayoutBinding& binding : m_Bindings)
	{
		binding.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.pNext = pNext;
	info.pBindings = m_Bindings.data();
	info.bindingCount = (uint32_t)m_Bindings.size();
	info.flags = flags;

	VkDescriptorSetLayout set;
	VULKAN_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

	return set;
}

void DescriptorWriter::WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
{
	VkDescriptorImageInfo& info = m_ImageInfos.emplace_back(VkDescriptorImageInfo{});
	info.sampler = sampler;
	info.imageView = image;
	info.imageLayout = layout;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &info;

	m_Writes.push_back(write);
}

void DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type)
{
	VkDescriptorBufferInfo& info = m_BufferInfos.emplace_back(VkDescriptorBufferInfo{});
	info.buffer = buffer;
	info.offset = offset;
	info.range = size;

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstBinding = binding;
	write.dstSet = VK_NULL_HANDLE;
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = &info;

	m_Writes.push_back(write);
}

void DescriptorWriter::Clear()
{
	m_ImageInfos.clear();
	m_Writes.clear();
	m_BufferInfos.clear();
}

void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set)
{
	for (VkWriteDescriptorSet& write : m_Writes)
	{
		write.dstSet = set;
	}
	vkUpdateDescriptorSets(device, (uint32_t)m_Writes.size(), m_Writes.data(), 0, nullptr);
}