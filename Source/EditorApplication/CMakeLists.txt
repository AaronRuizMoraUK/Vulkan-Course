cmake_minimum_required(VERSION 3.28)

file(GLOB_RECURSE EDITOR_APPLICATION_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/Source/EditorApplication/Source/*.*")

source_group(TREE "${CMAKE_SOURCE_DIR}/Source/EditorApplication" FILES 
    ${EDITOR_APPLICATION_SOURCE_FILES})

add_executable(EditorApplication
    ${EDITOR_APPLICATION_SOURCE_FILES})

# Includes
target_include_directories(EditorApplication PRIVATE "${CMAKE_SOURCE_DIR}/Source/EditorApplication/Source")

# Libraries
target_link_libraries(EditorApplication PRIVATE Runtime)

# Set warning levels based on the compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(EditorApplication PRIVATE -Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(EditorApplication PRIVATE /W4 /WX)
endif()
