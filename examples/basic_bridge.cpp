// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

/**
 * Basic ThinQ Bridge Example
 * 
 * This example demonstrates the minimal setup required to create
 * a ThinQ to Matter bridge.
 */

#include <chrono>
#include <memory>
#include <thread>

#include <spdlog/spdlog.h>

#include "thinq_proxy/auth.hpp"
#include "thinq_proxy/api_client.hpp"
#include "thinq_proxy/device.hpp"
#include "thinq_proxy/matter_bridge.hpp"

using namespace thinq_proxy;

int main() {
    // 1. Set up authentication
    // Replace with your actual credentials from LG ThinQ Developer Portal
    auto auth = std::make_shared<Auth>(
        "YOUR_PAT_TOKEN",           // Personal Access Token
        "YOUR_CLIENT_ID",            // Client ID
        CountryCode::US              // Country code
    );

    // 2. Create API client
    auto api_client = std::make_shared<ApiClient>(auth);

    // 3. Discover devices
    auto devices_result = api_client->get_devices();
    if (!devices_result) {
        spdlog::error("Failed to get devices: {}", devices_result.error().to_string());
        return 1;
    }

    spdlog::info("Found {} device(s)", devices_result.value().size());

    // 4. Create device objects
    std::vector<std::shared_ptr<Device>> devices;
    for (const auto& info : devices_result.value()) {
        spdlog::info("  - {}", info.device_name);
        devices.push_back(create_device(info, api_client));
    }

    // 5. Create Matter bridge
    MatterBridge::Config bridge_config;
    bridge_config.bridge_name = "My ThinQ Bridge";
    
    MatterBridge bridge(bridge_config);

    // 6. Initialize bridge
    auto init_result = bridge.initialize();
    if (!init_result) {
        spdlog::error("Failed to initialize bridge: {}", init_result.error().to_string());
        return 1;
    }

    // 7. Add devices to bridge
    for (auto& device : devices) {
        auto add_result = bridge.add_device(device);
        if (!add_result) {
            spdlog::error("Failed to add device: {}", add_result.error().to_string());
            continue;
        }
        spdlog::info("Added device as endpoint {}", add_result.value());
    }

    // 8. Start bridge
    auto start_result = bridge.start();
    if (!start_result) {
        spdlog::error("Failed to start bridge: {}", start_result.error().to_string());
        return 1;
    }

    spdlog::info("\nBridge is running. Press Ctrl+C to stop.");

    // 9. Main loop
    while (true) {
        // Sync state every 5 seconds
        bridge.sync_state();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
