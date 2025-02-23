cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
        GLFW
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        GIT_TAG "3.4"
        GIT_PROGRESS TRUE
)

# FetchContent_Declare(
#         vulkan
#         GIT_REPOSITORY "https://github.com/KhronosGroup/Vulkan-Headers.git"
#         GIT_TAG "v1.4.304"
# )

set(GLM_ENABLE_CXX_20 ON)
set(GLM_BUILD_LIBRARY OFF)
set(GLM_ENABLE_FAST_MATH ON)
FetchContent_Declare(
        glm
        GIT_REPOSITORY "https://github.com/g-truc/glm.git"
        GIT_TAG "1.0.1"
)

set(IMGUI_GLFW ON)
set(IMGUI_VULKAN ON)
set(IMGUI_GLFW ON)
set(IMGUI_STDLIB ON)
FetchContent_Declare(
        ImGui
        GIT_REPOSITORY "https://github.com/Sayama3/imgui.git"
        GIT_TAG "docking"
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        Spdlog
        GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
        GIT_TAG "v1.15.1"
        GIT_PROGRESS TRUE
)

set(TRACY_ENABLE ON)
set(TRACY_STATIC ON)
set(TRACY_ON_DEMAND ON)
FetchContent_Declare(
        Tracy
        GIT_REPOSITORY "https://github.com/wolfpld/tracy.git"
        GIT_TAG "v0.11.1"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(GLFW glm ImGui Spdlog Tracy)
find_package(Vulkan REQUIRED COMPONENTS SPIRV-Tools glslc glslang shaderc_combined)

# Need to say to imgui where is glfw & Vulkan Headers
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)