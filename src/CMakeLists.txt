add_executable(engine 
  main.cpp
 "VulkanRenderer.h" "VulkanRenderer.cpp" "VulkanUtils.cpp" "VulkanUtils.h" "VulkanDescriptor.cpp" "VulkanDescriptor.h" "Shader.cpp" "Shader.h" "PipelineBuilder.cpp" "PipelineBuilder.h" "IRenderable.h" "Mesh.cpp" "Mesh.h" "RenderPoint.cpp" "RenderPoint.h" "Vertex.h" "VulkanTypes.h" "PointRenderer.cpp" "PointRenderer.h" "MeshRenderer.cpp" "MeshRenderer.h")

set_property(TARGET engine PROPERTY CXX_STANDARD 23)
target_compile_definitions(engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/third_party/imgui/**.h")

#target_link_libraries(engine PUBLIC vma glm Vulkan::Vulkan fmt::fmt stb_image SDL2::SDL2 vkbootstrap imgui fastgltf::fastgltf)
target_link_libraries(engine PUBLIC glm::glm-header-only imgui SDL3::SDL3 vk-bootstrap::vk-bootstrap Vulkan::Vulkan VulkanMemoryAllocator)