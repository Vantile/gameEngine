find_package(Vulkan REQUIRED)

add_subdirectory(glm)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
add_subdirectory(vk-bootstrap)
add_subdirectory(VulkanMemoryAllocator)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC "imgui" "imgui/backends")
target_sources(imgui PRIVATE
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	#imgui/imgui_internal.h
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/backends/imgui_impl_sdl3.cpp
	imgui/backends/imgui_impl_sdl3.h
	imgui/backends/imgui_impl_vulkan.cpp
	imgui/backends/imgui_impl_vulkan.h
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL3::SDL3)