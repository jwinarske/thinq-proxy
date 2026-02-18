cmake_minimum_required(VERSION 3.25)
project(thinq-proxy VERSION 0.1.0 LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(CURL REQUIRED)

# Use FetchContent for dependencies
include(FetchContent)

# Fetch glaze for JSON serialization
FetchContent_Declare(
    glaze
    GIT_REPOSITORY https://github.com/stephenberry/glaze.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

# Library sources
set(LIB_SOURCES
    src/thinq/auth.cpp
    src/thinq/device.cpp
    src/thinq/api_client.cpp
    src/matter/bridge.cpp
    src/devices/air_conditioner.cpp
    src/devices/washer.cpp
    src/devices/refrigerator.cpp
    src/config/config.cpp
)

# Create library
add_library(thinq_proxy_lib STATIC ${LIB_SOURCES})

target_include_directories(thinq_proxy_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(thinq_proxy_lib
    PUBLIC
        CURL::libcurl
        glaze::glaze
)

# Set compile options
target_compile_options(thinq_proxy_lib PRIVATE
    -Wall
    -Wextra
    -pedantic
)

# Main executable
add_executable(thinq-proxy src/main.cpp)

target_link_libraries(thinq-proxy
    PRIVATE
        thinq_proxy_lib
)

# Installation rules
install(TARGETS thinq-proxy thinq_proxy_lib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# Optional: Enable testing
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
