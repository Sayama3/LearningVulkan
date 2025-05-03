cmake_minimum_required(VERSION 3.25)

find_package(Vulkan REQUIRED)

add_executable(Application
    include/stb_image.h
    src/main.cpp
)

target_include_directories(Application PUBLIC include)
target_include_directories(Application PRIVATE src)

target_link_libraries(Application PUBLIC
	glfw
    Vulkan::Vulkan
	glm::glm-header-only
	imgui
	spdlog::spdlog
	TracyClient
)

# Might have to do something with that
#message(FATAL_ERROR "$ENV{VULKAN_SDK}/Bin/glslc")

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

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
	file(CREATE_LINK ${CMAKE_SOURCE_DIR}/Assets/ ${CMAKE_CURRENT_BINARY_DIR}/Assets/ RESULT copy_result COPY_ON_ERROR SYMBOLIC)
	if(NOT (copy_result EQUAL 0))
		message(WARNING
				"Copy made instead of symlink.\n"
				"${copy_result}"
		)
	endif()
else()
	file(COPY ${CMAKE_SOURCE_DIR}/Assets/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets/)
endif()