#include "thinq_proxy/api_client.hpp"
#include "thinq_proxy/auth.hpp"
#include "thinq_proxy/config.hpp"
#include "thinq_proxy/matter_bridge.hpp"
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        running = false;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "ThinQ Connect to Matter Bridge\n";
    std::cout << "================================\n\n";
    
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Load configuration
    std::string config_file = "config.txt";
    if (argc > 1) {
        config_file = argv[1];
    }
    
    std::cout << "Loading configuration from: " << config_file << "\n";
    auto config_result = thinq_proxy::Config::load(config_file);
    if (!config_result) {
        std::cerr << "Failed to load configuration: " 
                  << config_result.error().message() << "\n";
        std::cerr << "\nExample configuration file format:\n";
        std::cerr << "  pat_token=YOUR_TOKEN_HERE\n";
        std::cerr << "  client_id=YOUR_CLIENT_ID\n";
        std::cerr << "  country=US\n";
        return 1;
    }
    
    auto config = std::move(config_result.value());
    std::cout << "Configuration loaded successfully\n";
    std::cout << "Country: " << thinq_proxy::country_to_string(config.country) << "\n\n";
    
    // Initialize ThinQ API client
    auto auth = std::make_shared<thinq_proxy::Auth>(
        config.pat_token,
        config.country,
        config.client_id
    );
    
    auto thinq_client = std::make_shared<thinq_proxy::ApiClient>(auth);
    
    std::cout << "Initializing ThinQ API client...\n";
    auto init_result = thinq_client->initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize ThinQ client: " 
                  << init_result.error().message() << "\n";
        return 1;
    }
    std::cout << "ThinQ API client initialized\n\n";
    
    // Discover devices
    std::cout << "Discovering devices...\n";
    auto devices_result = thinq_client->get_devices();
    if (!devices_result) {
        std::cerr << "Failed to get devices: " 
                  << devices_result.error().message() << "\n";
        return 1;
    }
    
    auto devices = std::move(devices_result.value());
    std::cout << "Found " << devices.size() << " device(s)\n";
    for (const auto& device : devices) {
        std::cout << "  - " << device->get_alias() 
                  << " (" << thinq_proxy::device_type_to_string(device->get_type()) << ")\n";
    }
    std::cout << "\n";
    
    // Initialize Matter bridge
    std::cout << "Initializing Matter bridge...\n";
    auto matter_bridge = std::make_shared<thinq_proxy::MatterBridge>();
    
    auto matter_init = matter_bridge->initialize();
    if (!matter_init) {
        std::cerr << "Failed to initialize Matter bridge: " 
                  << matter_init.error().message() << "\n";
        return 1;
    }
    std::cout << "Matter bridge initialized\n\n";
    
    // Add ThinQ devices to Matter bridge
    std::cout << "Adding devices to Matter bridge...\n";
    for (const auto& device : devices) {
        auto add_result = matter_bridge->add_device(device);
        if (!add_result) {
            std::cerr << "Failed to add device " << device->get_alias() 
                      << ": " << add_result.error().message() << "\n";
        } else {
            std::cout << "  Added: " << device->get_alias() << "\n";
        }
    }
    std::cout << "\n";
    
    // Start Matter bridge
    std::cout << "Starting Matter bridge...\n";
    auto start_result = matter_bridge->start();
    if (!start_result) {
        std::cerr << "Failed to start Matter bridge: " 
                  << start_result.error().message() << "\n";
        return 1;
    }
    std::cout << "Matter bridge started successfully\n";
    std::cout << "Bridge is now running. Press Ctrl+C to stop.\n\n";
    
    // Main loop - poll ThinQ API and update Matter attributes
    while (running) {
        // Poll device updates
        for (const auto& device : devices) {
            auto status = thinq_client->get_device_status(device->get_id());
            if (status) {
                device->update_status(status.value());
            }
        }
        
        // Sync state with Matter
        matter_bridge->sync_state();
        
        // Wait for next poll interval
        std::this_thread::sleep_for(config.poll_interval);
    }
    
    std::cout << "\nShutting down...\n";
    matter_bridge->stop();
    std::cout << "Bridge stopped. Goodbye!\n";
    
    return 0;
}
