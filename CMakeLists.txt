cmake_minimum_required(VERSION 3.25)
project(thinq-proxy VERSION 0.1.0 LANGUAGES CXX)

# C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find required packages
find_package(CURL REQUIRED)

# Optional: Matter SDK integration
# The connectedhomeip Matter SDK must be built separately due to its complexity.
# See README.md for Matter SDK build instructions.
option(ENABLE_MATTER_SDK "Enable Matter SDK integration (requires pre-built SDK)" OFF)

if(ENABLE_MATTER_SDK)
    # Try to find Matter SDK
    # Users should set MATTER_SDK_PATH or CMAKE_PREFIX_PATH to the Matter SDK installation
    find_path(MATTER_INCLUDE_DIR
        NAMES app/server/Server.h
        HINTS
            ${MATTER_SDK_PATH}/include
            ${CMAKE_PREFIX_PATH}/include
            /usr/local/include/matter
            /usr/include/matter
        PATH_SUFFIXES chip
    )
    
    find_library(MATTER_CORE_LIB
        NAMES libCHIPCoreStaticLibrary.a CHIPCoreStaticLibrary
        HINTS
            ${MATTER_SDK_PATH}/lib
            ${CMAKE_PREFIX_PATH}/lib
            /usr/local/lib
            /usr/lib
    )
    
    if(MATTER_INCLUDE_DIR AND MATTER_CORE_LIB)
        message(STATUS "Found Matter SDK:")
        message(STATUS "  Include dir: ${MATTER_INCLUDE_DIR}")
        message(STATUS "  Core library: ${MATTER_CORE_LIB}")
        
        # Get the parent directory for includes
        get_filename_component(MATTER_INCLUDE_BASE ${MATTER_INCLUDE_DIR} DIRECTORY)
        
        set(MATTER_FOUND TRUE)
        add_definitions(-DHAVE_MATTER_SDK=1)
        
        # Add Matter SDK libraries (simplified - may need adjustment based on actual SDK)
        set(MATTER_LIBRARIES ${MATTER_CORE_LIB})
    else()
        message(WARNING "Matter SDK not found. Building without Matter support.")
        message(WARNING "To enable Matter SDK, set MATTER_SDK_PATH or install SDK to standard location.")
        set(MATTER_FOUND FALSE)
    endif()
else()
    message(STATUS "Matter SDK integration disabled (use -DENABLE_MATTER_SDK=ON to enable)")
    set(MATTER_FOUND FALSE)
endif()

# spdlog - Structured logging via git submodule
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "Use std::format instead of bundled fmt" FORCE)
add_subdirectory(third_party/spdlog)

# FetchContent for dependencies
include(FetchContent)

# Glaze - Modern C++ JSON library with reflection
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

# Library sources
set(THINQ_PROXY_SOURCES
    src/thinq/auth.cpp
    src/thinq/api_client.cpp
    src/thinq/device.cpp
    src/matter/bridge.cpp
    src/devices/air_conditioner.cpp
    src/devices/washer.cpp
    src/devices/refrigerator.cpp
    src/config/config.cpp
)

# Create library
add_library(thinq_proxy_lib STATIC ${THINQ_PROXY_SOURCES})

target_include_directories(thinq_proxy_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Add Matter SDK include directories if available
if(MATTER_FOUND)
    target_include_directories(thinq_proxy_lib
        PRIVATE
            ${MATTER_INCLUDE_BASE}
    )
endif()

target_link_libraries(thinq_proxy_lib
    PUBLIC
        CURL::libcurl
        glaze::glaze
        spdlog::spdlog
)

# Link Matter SDK libraries if available
if(MATTER_FOUND)
    target_link_libraries(thinq_proxy_lib
        PRIVATE
            ${MATTER_LIBRARIES}
    )
endif()

# Main executable
add_executable(thinq-proxy src/main.cpp)

target_link_libraries(thinq-proxy
    PRIVATE
        thinq_proxy_lib
)

# Install targets
install(TARGETS thinq-proxy thinq_proxy_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# Examples (optional)
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "=== ThinQ Proxy Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CURL found: ${CURL_FOUND}")
message(STATUS "Matter SDK enabled: ${ENABLE_MATTER_SDK}")
if(ENABLE_MATTER_SDK)
    message(STATUS "Matter SDK found: ${MATTER_FOUND}")
endif()
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
message(STATUS "================================")
message(STATUS "")
