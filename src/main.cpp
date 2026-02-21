// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <mutex>
#include <thread>

#include <spdlog/spdlog.h>

#include "thinq_proxy/config.hpp"
#include "thinq_proxy/auth.hpp"
#include "thinq_proxy/api_client.hpp"
#include "thinq_proxy/device.hpp"
#include "thinq_proxy/matter_bridge.hpp"
#include "glaze/glaze.hpp"

using namespace thinq_proxy;

std::atomic<bool> running{true};
std::condition_variable shutdown_cv;
std::mutex shutdown_mutex;

void print_device_status(ApiClient& client, const DeviceInfo& device_info) {
    auto status_result = client.get_device_status(device_info.device_id);
    if (!status_result) {
        spdlog::error("    status error: {}", status_result.error().to_string());
        return;
    }

    const auto& status = status_result.value();
    spdlog::info("    status: online={}, type={}, last_update_ms={}",
                 (status.online ? "true" : "false"),
                 to_string(status.type),
                 status.last_update_ms);
}

void print_device_profile(ApiClient& client, const DeviceInfo& device_info) {
    auto profile_result = client.get_device_profile(device_info.device_id);
    if (!profile_result) {
        spdlog::error("    profile error: {}", profile_result.error().to_string());
        return;
    }

    std::string pretty_profile;
    glz::prettify_json(profile_result.value(), pretty_profile);
    if (pretty_profile.empty()) {
        spdlog::info("    profile: {}", profile_result.value());
        return;
    }

    spdlog::info("    profile:\n{}", pretty_profile);
}

void signal_handler(int signal) {
    spdlog::info("\nReceived signal {}, shutting down...", signal);
    running = false;
    shutdown_cv.notify_all();
}

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    spdlog::info("ThinQ to Matter Bridge v0.1.0");
    spdlog::info("Copyright 2024 ThinQ Proxy Contributors");

    // Set up signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 1. Load configuration
    std::string config_path = "config.json";
    if (argc > 1) {
        config_path = argv[1];
    }

    spdlog::info("Loading configuration from {}...", config_path);
    auto config_result = Config::load(config_path);
    if (!config_result) {
        spdlog::error("Error loading configuration: {}", config_result.error().to_string());
        spdlog::error("Please create a config.json file with your credentials.");
        spdlog::error("See examples/config_example.json for reference.");
        return 1;
    }

    auto config = config_result.value();
    if (!config.is_valid()) {
        spdlog::error("Invalid configuration: missing required fields");
        return 1;
    }

    // 2. Initialize ThinQ API client
    spdlog::info("Initializing ThinQ API client...");
    auto auth = std::make_shared<Auth>(
        config.thinq_config.pat_token,
        config.thinq_config.client_id,
        config.thinq_config.country
    );

    auto thinq_client = std::make_shared<ApiClient>(auth);
    thinq_client->set_timeout(30000); // 30 second timeout

    // 3. Discover devices
    spdlog::info("Discovering ThinQ devices...");
    auto devices_result = thinq_client->get_devices();
    if (!devices_result) {
        spdlog::error("Error discovering devices: {}", devices_result.error().to_string());
        return 1;
    }

    auto device_infos = devices_result.value();
    spdlog::info("Found {} device(s)", device_infos.size());

    if (device_infos.empty()) {
        spdlog::info("No devices found. Make sure you have ThinQ devices registered.");
        return 0;
    }

    // Create device objects
    std::vector<std::shared_ptr<Device>> devices;
    for (const auto& info : device_infos) {
        spdlog::info("  - {} ({})", info.device_name, to_string(info.type));
        print_device_status(*thinq_client, info);
        print_device_profile(*thinq_client, info);
        devices.push_back(create_device(info, thinq_client));
    }

    // 4. Initialize Matter bridge
    spdlog::info("\nInitializing Matter bridge...");
    MatterBridge matter_bridge(config.matter_config);
    
    auto init_result = matter_bridge.initialize();
    if (!init_result) {
        spdlog::error("Error initializing Matter bridge: {}", init_result.error().to_string());
        return 1;
    }

    // 5. Add ThinQ devices to Matter bridge
    spdlog::info("Adding devices to Matter bridge...");
    for (const auto& device : devices) {
        auto endpoint_result = matter_bridge.add_device(device);
        if (!endpoint_result) {
            spdlog::warn("Failed to add device {}: {}",
                          device->device_name(), endpoint_result.error().to_string());
            continue;
        }
        spdlog::info("  - Added {} as endpoint {}", device->device_name(), endpoint_result.value());
    }

    // 6. Start bridge
    spdlog::info("\nStarting Matter bridge...");
    auto start_result = matter_bridge.start();
    if (!start_result) {
        spdlog::error("Error starting Matter bridge: {}", start_result.error().to_string());
        return 1;
    }

    spdlog::info("\n=== Bridge Ready ===");
    spdlog::info("Bridge Name: {}", config.matter_config.bridge_name);
    spdlog::info("Setup Code: {}", config.matter_config.setup_passcode);
    spdlog::info("Discriminator: {}", config.matter_config.discriminator);
    spdlog::info("Port: {}", config.matter_config.port);
    spdlog::info("\nCommission this bridge using your Matter controller.");
    spdlog::info("Press Ctrl+C to stop.");

    // 7. Main loop - poll ThinQ API and sync Matter state
    auto poll_interval = std::chrono::seconds(config.thinq_config.poll_interval_sec);
    int cycle = 0;

    while (running) {
        cycle++;
        spdlog::info("Sync cycle {}...", cycle);

        // Sync state between ThinQ and Matter
        auto sync_result = matter_bridge.sync_state();
        if (!sync_result) {
            spdlog::warn("Sync failed: {}", sync_result.error().to_string());
        }

        // Wait for either timeout or shutdown signal
        std::unique_lock<std::mutex> lock(shutdown_mutex);
        shutdown_cv.wait_for(lock, poll_interval, [] { return !running.load(); });
    }

    // Clean shutdown
    spdlog::info("\nStopping Matter bridge...");
    matter_bridge.stop();
    spdlog::info("Shutdown complete.");

    return 0;
}
