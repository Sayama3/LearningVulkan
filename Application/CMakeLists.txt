cmake_minimum_required(VERSION 3.25)

find_package(Vulkan REQUIRED COMPONENTS SPIRV-Tools glslc glslang shaderc_combined)

add_executable(Application
    src/main.cpp
)

target_link_libraries(Application PUBLIC
	glfw
    Vulkan::Vulkan
	glm::glm-header-only
	imgui
	spdlog::spdlog
	TracyClient
)
