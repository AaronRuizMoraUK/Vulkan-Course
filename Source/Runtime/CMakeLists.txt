cmake_minimum_required(VERSION 3.28)

file(GLOB_RECURSE RUNTIME_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/Source/Runtime/Source/*.*")

source_group(TREE "${CMAKE_SOURCE_DIR}/Source/Runtime" FILES ${RUNTIME_SOURCE_FILES})

add_library(Runtime STATIC ${RUNTIME_SOURCE_FILES})

set_target_properties(Runtime PROPERTIES FOLDER "Engine")

# Includes
target_include_directories(Runtime PUBLIC "${CMAKE_SOURCE_DIR}/Source/Runtime/Source")

# Compiler definitions
target_compile_definitions(Runtime PRIVATE STB_IMAGE_IMPLEMENTATION=1) # For stb

# Libraries
target_link_libraries(Runtime PUBLIC Core)
target_link_libraries(Runtime PRIVATE Graphics)
target_link_libraries(Runtime PRIVATE glfw)
target_link_libraries(Runtime PRIVATE stb)
target_link_libraries(Runtime PRIVATE assimp)

# Set warning levels based on the compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Runtime PRIVATE -Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Runtime PRIVATE /W4 /WX)
endif()
