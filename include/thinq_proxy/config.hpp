#pragma once

#include "error.hpp"
#include "types.hpp"
#include <string>
#include <chrono>

namespace thinq_proxy {

/**
 * @brief Configuration for ThinQ Proxy
 */
struct Config {
    // ThinQ API credentials
    std::string pat_token;
    Country country{Country::US};
    std::string client_id;
    
    // Matter configuration
    uint16_t matter_vendor_id{0xFFF1};
    uint16_t matter_product_id{0x8000};
    std::string matter_device_name{"ThinQ Bridge"};
    
    // Polling configuration
    std::chrono::seconds poll_interval{5};
    
    /**
     * @brief Load configuration from JSON file
     * @param filename Path to config file
     */
    static Result<Config> load(const std::string& filename);
    
    /**
     * @brief Save configuration to JSON file
     * @param filename Path to config file
     */
    Result<void> save(const std::string& filename) const;
    
    /**
     * @brief Validate configuration
     */
    Result<void> validate() const;
};

} // namespace thinq_proxy
