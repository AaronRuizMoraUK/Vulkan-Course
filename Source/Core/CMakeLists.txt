cmake_minimum_required(VERSION 3.28)

file(GLOB_RECURSE CORE_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/Source/Core/Source/*.*")

source_group(TREE "${CMAKE_SOURCE_DIR}/Source/Core" FILES ${CORE_SOURCE_FILES})

add_library(Core STATIC ${CORE_SOURCE_FILES})

set_target_properties(Core PROPERTIES FOLDER "Engine")

# Includes
target_include_directories(Core PUBLIC "${CMAKE_SOURCE_DIR}/Source/Core/Source")

# Libraries
target_link_libraries(Core PUBLIC mathfu)

# Set warning levels based on the compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(Core PRIVATE -Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(Core PRIVATE /W4 /WX)
endif()
