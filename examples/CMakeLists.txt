cmake_minimum_required(VERSION 3.25)

# Basic bridge example
add_executable(basic_bridge basic_bridge.cpp)
target_link_libraries(basic_bridge PRIVATE thinq_proxy_lib)

# Install examples
install(TARGETS basic_bridge
    RUNTIME DESTINATION bin/examples
)

install(FILES config_example.json
    DESTINATION share/thinq-proxy/examples
)
