add_executable(engine 
  main.cpp
 "renderer/VulkanRenderer.h" "renderer/VulkanRenderer.cpp" "renderer/VulkanUtils.cpp" "renderer/VulkanUtils.h" "renderer/VulkanDescriptor.cpp" "renderer/VulkanDescriptor.h" "renderer/Shader.cpp" "renderer/Shader.h" "renderer/PipelineBuilder.cpp" "renderer/PipelineBuilder.h" "renderer/IRenderable.h" "renderer/Mesh.cpp" "renderer/Mesh.h" "renderer/RenderPoint.cpp" "renderer/RenderPoint.h" "renderer/Vertex.h" "renderer/VulkanTypes.h" "renderer/PointRenderer.cpp" "renderer/PointRenderer.h" "renderer/MeshRenderer.cpp" "renderer/MeshRenderer.h" "renderer/VulkanMemoryManager.cpp" "renderer/VulkanMemoryManager.h" "entitysystem/Entity.cpp" "entitysystem/Entity.h" "entitysystem/component/Component.cpp" "entitysystem/component/Component.h" "entitysystem/system/System.cpp" "entitysystem/system/System.h" "entitysystem/ECSManager.cpp" "entitysystem/ECSManager.h" "entitysystem/EntityPool.cpp" "entitysystem/EntityPool.h" "entitysystem/SparseSet.inl" "entitysystem/SparseSet.h" "entitysystem/EntityRegistry.h" "entitysystem/EntityRegistry.inl" "entitysystem/View.h" "entitysystem/View.cpp" "entitysystem/component/TransformComponent.h" "entitysystem/component/RenderableComponent.h" "entitysystem/system/MovementSystem.h" "entitysystem/system/RenderSystem.h" "engine/Engine.h" "engine/Engine.cpp" "engine/FrameData.h" "engine/Job.h" "engine/Job.cpp")

set_property(TARGET engine PROPERTY CXX_STANDARD 23)
target_compile_definitions(engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE TRACY_ENABLE TRACY_ON_DEMAND)
target_sources(engine PRIVATE "${PROJECT_SOURCE_DIR}/third_party/tracy/public/TracyClient.cpp")
set_source_files_properties("${PROJECT_SOURCE_DIR}/third_party/tracy/public/TracyClient.cpp" PROPERTIES COMPILE_FLAGS "-w")
target_include_directories(engine SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/third_party/imgui/**.h" "${PROJECT_SOURCE_DIR}/third_party/tracy/public")
target_compile_definitions(engine PRIVATE TRACY_ENABLE TRACY_ON_DEMAND)

target_link_libraries(engine PUBLIC glm::glm-header-only imgui SDL3::SDL3 vk-bootstrap::vk-bootstrap Vulkan::Vulkan VulkanMemoryAllocator)

function(suppress_warnings TARGET)
    if(TARGET ${TARGET})
        # Resolve alias to the real target
        get_target_property(ALIAS ${TARGET} ALIASED_TARGET)
        if(ALIAS)
            set(REAL_TARGET ${ALIAS})
        else()
            set(REAL_TARGET ${TARGET})
        endif()

        get_target_property(INCS ${REAL_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
        if(INCS)
            set_target_properties(${REAL_TARGET} PROPERTIES
                INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${INCS}"
            )
        endif()
    endif()
endfunction()

suppress_warnings(glm::glm-header-only)
suppress_warnings(imgui)
suppress_warnings(SDL3::SDL3)
suppress_warnings(vk-bootstrap::vk-bootstrap)
suppress_warnings(Vulkan::Vulkan)
suppress_warnings(VulkanMemoryAllocator)