find_package(Vulkan REQUIRED)

add_subdirectory(glm)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(vk-bootstrap)
add_subdirectory(VulkanMemoryAllocator)