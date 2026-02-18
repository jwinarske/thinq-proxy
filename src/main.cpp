// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/config.hpp"
#include "thinq_proxy/auth.hpp"
#include "thinq_proxy/api_client.hpp"
#include "thinq_proxy/device.hpp"
#include "thinq_proxy/matter_bridge.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace thinq_proxy;

std::atomic<bool> running{true};
std::condition_variable shutdown_cv;
std::mutex shutdown_mutex;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down...\n";
    running = false;
    shutdown_cv.notify_all();
}

int main(int argc, char* argv[]) {
    std::cout << "ThinQ to Matter Bridge v0.1.0\n";
    std::cout << "Copyright 2024 ThinQ Proxy Contributors\n\n";

    // Set up signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 1. Load configuration
    std::string config_path = "config.json";
    if (argc > 1) {
        config_path = argv[1];
    }

    std::cout << "Loading configuration from " << config_path << "...\n";
    auto config_result = Config::load(config_path);
    if (!config_result) {
        std::cerr << "Error loading configuration: " 
                  << config_result.error().to_string() << "\n";
        std::cerr << "Please create a config.json file with your credentials.\n";
        std::cerr << "See examples/config_example.json for reference.\n";
        return 1;
    }

    auto config = config_result.value();
    if (!config.is_valid()) {
        std::cerr << "Invalid configuration: missing required fields\n";
        return 1;
    }

    // 2. Initialize ThinQ API client
    std::cout << "Initializing ThinQ API client...\n";
    auto auth = std::make_shared<Auth>(
        config.thinq_config.pat_token,
        config.thinq_config.client_id,
        config.thinq_config.country
    );

    auto thinq_client = std::make_shared<ApiClient>(auth);
    thinq_client->set_timeout(30000); // 30 second timeout

    // 3. Discover devices
    std::cout << "Discovering ThinQ devices...\n";
    auto devices_result = thinq_client->get_devices();
    if (!devices_result) {
        std::cerr << "Error discovering devices: " 
                  << devices_result.error().to_string() << "\n";
        return 1;
    }

    auto device_infos = devices_result.value();
    std::cout << "Found " << device_infos.size() << " device(s)\n";

    if (device_infos.empty()) {
        std::cout << "No devices found. Make sure you have ThinQ devices registered.\n";
        return 0;
    }

    // Create device objects
    std::vector<std::shared_ptr<Device>> devices;
    for (const auto& info : device_infos) {
        std::cout << "  - " << info.device_name 
                  << " (" << to_string(info.type) << ")\n";
        devices.push_back(create_device(info, thinq_client));
    }

    // 4. Initialize Matter bridge
    std::cout << "\nInitializing Matter bridge...\n";
    MatterBridge matter_bridge(config.matter_config);
    
    auto init_result = matter_bridge.initialize();
    if (!init_result) {
        std::cerr << "Error initializing Matter bridge: " 
                  << init_result.error().to_string() << "\n";
        return 1;
    }

    // 5. Add ThinQ devices to Matter bridge
    std::cout << "Adding devices to Matter bridge...\n";
    for (const auto& device : devices) {
        auto endpoint_result = matter_bridge.add_device(device);
        if (!endpoint_result) {
            std::cerr << "Warning: Failed to add device " 
                      << device->device_name() << ": "
                      << endpoint_result.error().to_string() << "\n";
            continue;
        }
        std::cout << "  - Added " << device->device_name() 
                  << " as endpoint " << endpoint_result.value() << "\n";
    }

    // 6. Start bridge
    std::cout << "\nStarting Matter bridge...\n";
    auto start_result = matter_bridge.start();
    if (!start_result) {
        std::cerr << "Error starting Matter bridge: " 
                  << start_result.error().to_string() << "\n";
        return 1;
    }

    std::cout << "\n=== Bridge Ready ===\n";
    std::cout << "Bridge Name: " << config.matter_config.bridge_name << "\n";
    std::cout << "Setup Code: " << config.matter_config.setup_passcode << "\n";
    std::cout << "Discriminator: " << config.matter_config.discriminator << "\n";
    std::cout << "Port: " << config.matter_config.port << "\n";
    std::cout << "\nCommission this bridge using your Matter controller.\n";
    std::cout << "Press Ctrl+C to stop.\n\n";

    // 7. Main loop - poll ThinQ API and sync Matter state
    auto poll_interval = std::chrono::seconds(config.thinq_config.poll_interval_sec);
    int cycle = 0;

    while (running) {
        cycle++;
        std::cout << "Sync cycle " << cycle << "...\n";

        // Sync state between ThinQ and Matter
        auto sync_result = matter_bridge.sync_state();
        if (!sync_result) {
            std::cerr << "Warning: Sync failed: " 
                      << sync_result.error().to_string() << "\n";
        }

        // Wait for either timeout or shutdown signal
        std::unique_lock<std::mutex> lock(shutdown_mutex);
        shutdown_cv.wait_for(lock, poll_interval, [] { return !running.load(); });
    }

    // Clean shutdown
    std::cout << "\nStopping Matter bridge...\n";
    matter_bridge.stop();
    std::cout << "Shutdown complete.\n";

    return 0;
}
