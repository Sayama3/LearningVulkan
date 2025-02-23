cmake_minimum_required(VERSION 3.25)
# Version with "dxc_lib", "dxc_exe", "volk"...

project(LearningVulkan VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_COMPUTED_DEFAULT "20")

option(LVK_NO_PROFILING "Stop the profiling of the application" OFF)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set(CPP_RTTI_ENABLED ON)

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(lib)
add_subdirectory(Application)

message(STATUS "Created target ${LIBRARY_TARGET_NAME}.")