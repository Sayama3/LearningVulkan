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

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
	file(CREATE_LINK ${CMAKE_SOURCE_DIR}/Shaders/ ${CMAKE_CURRENT_BINARY_DIR}/Shaders/ RESULT copy_result COPY_ON_ERROR SYMBOLIC)
	if(NOT (copy_result EQUAL 0))
		message(WARNING
				"Copy made instead of symlink.\n"
				"${copy_result}"
		)
	endif()
else()
	file(COPY ${CMAKE_SOURCE_DIR}/Shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Shaders/)
endif()