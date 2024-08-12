cmake_minimum_required(VERSION 3.28)

# Use C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
endif()

# Project name
project ("Vulkan-Course")

# ----------------------------------------------------
# Setup build configuration types

set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Release" CACHE STRING "Config types" FORCE)

# Copy Release configuration flags to Development configuration
set(CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "Flags used by the C++ compiler during Development builds." FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "${CMAKE_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used for linking binaries during Development builds." FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}" CACHE STRING "Flags used by the shared libraries linker during Development builds." FORCE)

# Adjust flags for Development configuration to allow debugging: remove NDEBUG macro and add debug information.
if(MSVC)
    string(REPLACE "/DNDEBUG" "/Zi" CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_DEVELOPMENT}")
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "${CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT} /DEBUG" CACHE STRING "Generate debug information" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT "${CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT} /DEBUG" CACHE STRING "Generate debug information" FORCE)
else()
    string(REPLACE "-DNDEBUG" "-g" CMAKE_CXX_FLAGS_DEVELOPMENT "${CMAKE_CXX_FLAGS_DEVELOPMENT}")
endif()

# ----------------------------------------------------
# 3rd party libraries

include("${CMAKE_SOURCE_DIR}/Scripts/FetchLibraries.cmake")

# ----------------------------------------------------
# Engine libraries

add_subdirectory(Source/Core)
add_subdirectory(Source/Runtime)

# ----------------------------------------------------
# EditorApplication project

add_subdirectory(Source/EditorApplication)

# Set EditorApplication as the default project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EditorApplication)

# ----------------------------------------------------
# Project with content from root folder for easy access from Visual Studio

set(ROOT_FILES "${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_SOURCE_DIR}/.gitignore")

file(GLOB_RECURSE SCRIPT_FILES
    "${CMAKE_SOURCE_DIR}/Scripts/*.*")

file(GLOB_RECURSE ASSET_FILES
    "${CMAKE_SOURCE_DIR}/Assets/*.*")
    
# Set the Asset files to be treated as non-compilable files
foreach(ASSET_FILE ${ASSET_FILES})
    set_source_files_properties(${ASSET_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
endforeach()

source_group(TREE "${CMAKE_SOURCE_DIR}" FILES 
    ${ROOT_FILES}
    ${SCRIPT_FILES}
    ${ASSET_FILES})

add_library(Content INTERFACE 
    ${ROOT_FILES}
    ${SCRIPT_FILES}
    ${ASSET_FILES})
