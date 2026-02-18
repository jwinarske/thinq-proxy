#pragma once

#include "auth.hpp"
#include "device.hpp"
#include "error.hpp"
#include <vector>
#include <memory>

namespace thinq_proxy {

/**
 * @brief ThinQ Connect API client
 * 
 * Handles all communication with LG ThinQ Connect REST API
 */
class ApiClient {
public:
    /**
     * @brief Construct API client
     * @param auth Authentication manager
     */
    explicit ApiClient(std::shared_ptr<Auth> auth);
    ~ApiClient();
    
    // Delete copy operations
    ApiClient(const ApiClient&) = delete;
    ApiClient& operator=(const ApiClient&) = delete;
    
    /**
     * @brief Initialize the client
     */
    Result<void> initialize();
    
    /**
     * @brief Get list of all devices
     */
    Result<std::vector<DevicePtr>> get_devices();
    
    /**
     * @brief Get status for a specific device
     * @param device_id Device ID
     */
    Result<std::string> get_device_status(const std::string& device_id);
    
    /**
     * @brief Send command to device
     * @param device_id Device ID
     * @param command Command name
     * @param parameters Command parameters as JSON
     */
    Result<void> send_device_command(const std::string& device_id,
                                     const std::string& command,
                                     const std::string& parameters);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace thinq_proxy
