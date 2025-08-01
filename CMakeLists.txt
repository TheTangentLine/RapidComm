cmake_minimum_required(VERSION 3.16)

# ================================ Project Configuration ====================================

project(http-ftp-server
    VERSION 1.0.0
    DESCRIPTION "Full-stack HTTP-FTP File Upload Server"
    LANGUAGES CXX
)

# ================================ C++ Standard and Features =================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ================================ Build Configuration ===================================

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# ================================ Compiler Flags ========================================

# Common flags for all build types
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# ================================ Dependencies ==========================================

# Find required packages
find_package(Threads REQUIRED)

# Check if we need to link filesystem library (required for older GCC versions)
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#include <filesystem>
int main() { 
    std::filesystem::path p; 
    return 0; 
}" FILESYSTEM_NO_LINK_NEEDED)

if(NOT FILESYSTEM_NO_LINK_NEEDED)
    # Try to find the filesystem library
    find_library(FILESYSTEM_LIB stdc++fs)
    if(NOT FILESYSTEM_LIB)
        find_library(FILESYSTEM_LIB c++fs)
    endif()
endif()

# ================================ Source Files ==========================================

# Define source files
set(SOURCES
    src/services/main.cpp
    src/services/http/HttpService.cpp
    src/services/socket/Socket.cpp
    src/services/ftp/FtpService.cpp
)

# Define header files (for IDE support)
set(HEADERS
    src/services/http/HttpService.hpp
    src/services/socket/Socket.hpp
    src/services/ftp/FtpService.hpp
)

# ================================ Executable Target ====================================

# Create the main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# ================================ Include Directories ==================================

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src/services
)

# ================================ Link Libraries =======================================

# Link required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Threads::Threads
)

# Link filesystem library if needed
if(FILESYSTEM_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${FILESYSTEM_LIB})
endif()

# ================================ Platform-specific Settings =======================

# Platform-specific configurations
if(APPLE)
    # macOS specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DARWIN_C_SOURCE)
elseif(UNIX AND NOT APPLE)
    # Linux specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GNU_SOURCE)
elseif(WIN32)
    # Windows specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# ================================ Output Directory =====================================

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ================================ Installation ========================================

# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install interface files
install(DIRECTORY src/interface/
    DESTINATION share/${PROJECT_NAME}/interface
    FILES_MATCHING PATTERN "*.html"
)

# ================================ Custom Targets ======================================

# Custom target to run the server
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMENT "Running HTTP-FTP Server from build/bin directory..."
)

# Custom target to clean build artifacts
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMENT "Cleaning all build artifacts..."
)

# ================================ Development Targets ==================================

# Target for code formatting (if clang-format is available)
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${SOURCES} ${HEADERS}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source code..."
    )
endif()

# ================================ Information Display ==================================

# Display build information
message(STATUS "")
message(STATUS "================================")
message(STATUS "  HTTP-FTP Server Build Info")
message(STATUS "================================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
if(FILESYSTEM_LIB)
    message(STATUS "Filesystem library: ${FILESYSTEM_LIB}")
else()
    message(STATUS "Filesystem library: built-in")
endif()
message(STATUS "================================")
message(STATUS "")
message(STATUS "Build commands:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
message(STATUS "  make run")
message(STATUS "")
message(STATUS "================================")