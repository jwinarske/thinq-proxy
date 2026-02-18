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

# Optional: Find Matter SDK
# Uncomment when Matter SDK is available
# find_package(matter CONFIG)
# if(NOT matter_FOUND)
#     find_package(PkgConfig REQUIRED)
#     pkg_check_modules(MATTER REQUIRED matter)
# endif()

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

target_link_libraries(thinq_proxy_lib
    PUBLIC
        CURL::libcurl
        glaze::glaze
    # Uncomment when Matter SDK is available
    # PRIVATE
    #     matter::matter
)

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
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
message(STATUS "================================")
message(STATUS "")
