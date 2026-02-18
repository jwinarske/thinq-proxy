#include "thinq_proxy/api_client.hpp"
#include "thinq_proxy/auth.hpp"
#include "thinq_proxy/matter_bridge.hpp"
#include "thinq_proxy/devices/air_conditioner.hpp"
#include <iostream>
#include <memory>

/**
 * Basic example showing how to use the ThinQ proxy library
 */
int main() {
    // Create authentication
    auto auth = std::make_shared<thinq_proxy::Auth>(
        "your_pat_token",
        thinq_proxy::Country::US,
        "your_client_id"
    );
    
    // Create API client
    auto client = std::make_shared<thinq_proxy::ApiClient>(auth);
    
    // Initialize
    auto init_result = client->initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize: " << init_result.error().message() << "\n";
        return 1;
    }
    
    // Get devices
    auto devices_result = client->get_devices();
    if (!devices_result) {
        std::cerr << "Failed to get devices: " << devices_result.error().message() << "\n";
        return 1;
    }
    
    // Create Matter bridge
    auto bridge = std::make_shared<thinq_proxy::MatterBridge>();
    bridge->initialize();
    
    // Add devices to bridge
    for (const auto& device : devices_result.value()) {
        bridge->add_device(device);
    }
    
    // Start bridge
    bridge->start();
    
    std::cout << "Bridge is running!\n";
    
    return 0;
}
