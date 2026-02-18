// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "device.hpp"
#include "error.hpp"
#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace thinq_proxy {

/**
 * @brief Matter bridge that exposes ThinQ devices via Matter protocol
 * 
 * This class acts as a coordinator between ThinQ devices and Matter endpoints.
 * It manages the Matter stack, device bridging, and state synchronization.
 */
class MatterBridge {
public:
    /// Configuration for Matter bridge
    struct Config {
        std::string bridge_name{"ThinQ Bridge"};
        uint16_t vendor_id{0xFFF1};  // Test vendor ID
        uint16_t product_id{0x8001};
        uint16_t discriminator{3840};
        uint32_t setup_passcode{20202021};
        uint16_t port{5540};
    };

    /**
     * @brief Construct Matter bridge with configuration
     * @param config Bridge configuration
     */
    explicit MatterBridge(Config config = {});
    
    /// Destructor
    ~MatterBridge();

    // Prevent copying
    MatterBridge(const MatterBridge&) = delete;
    MatterBridge& operator=(const MatterBridge&) = delete;

    /**
     * @brief Initialize Matter stack
     * @return Success or error
     */
    Expected<void> initialize();

    /**
     * @brief Add ThinQ device to bridge
     * @param device Device to add
     * @return Matter endpoint ID or error
     */
    Expected<uint16_t> add_device(std::shared_ptr<Device> device);

    /**
     * @brief Remove device from bridge
     * @param device_id ThinQ device ID
     * @return Success or error
     */
    Expected<void> remove_device(const std::string& device_id);

    /**
     * @brief Start bridge (commission, advertise)
     * @return Success or error
     */
    Expected<void> start();

    /**
     * @brief Stop bridge
     * @return Success or error
     */
    Expected<void> stop();

    /**
     * @brief Synchronize state between ThinQ and Matter
     * 
     * Polls ThinQ devices and updates Matter attributes
     * @return Success or error
     */
    Expected<void> sync_state();

    /**
     * @brief Check if bridge is running
     */
    bool is_running() const { return running_; }

    /**
     * @brief Get list of bridged device IDs
     */
    std::vector<std::string> get_bridged_devices() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    
    Config config_;
    std::atomic<bool> running_{false};
    
    std::map<std::string, std::shared_ptr<Device>> devices_;
    std::map<std::string, uint16_t> device_endpoints_;
};

} // namespace thinq_proxy
