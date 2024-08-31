cmake_minimum_required(VERSION 3.28)

# Requires Vulkan 1.3 SDK to be installed. It can be downloaded from https://vulkan.lunarg.com/
find_package(Vulkan 1.3 REQUIRED)

file(GLOB_RECURSE GRAPHICS_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/Source/Graphics/Source/*.*")

source_group(TREE "${CMAKE_SOURCE_DIR}/Source/Graphics" FILES ${GRAPHICS_SOURCE_FILES})

add_library(Graphics STATIC ${GRAPHICS_SOURCE_FILES})

set_target_properties(Graphics PROPERTIES FOLDER "Engine")

# Includes
target_include_directories(Graphics PUBLIC "${CMAKE_SOURCE_DIR}/Source/Graphics/Source")

# Libraries
target_link_libraries(Graphics PUBLIC Core)
target_link_libraries(Graphics PUBLIC Vulkan::Vulkan) # TODO: Make private when Runtime Renderer doesn't use Vulkan directly
target_link_libraries(Graphics PRIVATE glfw)

# Set warning levels based on the compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Graphics PRIVATE -Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Graphics PRIVATE /W4 /WX)
endif()
