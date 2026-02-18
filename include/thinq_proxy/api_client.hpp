// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "auth.hpp"
#include "types.hpp"
#include "error.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace thinq_proxy {

/**
 * @brief HTTP REST client for ThinQ Connect API
 * 
 * Provides methods to interact with the LG ThinQ Connect API including
 * device discovery, status retrieval, and control commands.
 * Uses libcurl for HTTP operations.
 */
class ApiClient {
public:
    /**
     * @brief Construct API client with authentication
     * @param auth Authentication credentials
     */
    explicit ApiClient(std::shared_ptr<Auth> auth);
    
    /// Destructor
    ~ApiClient();

    // Prevent copying
    ApiClient(const ApiClient&) = delete;
    ApiClient& operator=(const ApiClient&) = delete;
    
    // Allow moving
    ApiClient(ApiClient&&) noexcept;
    ApiClient& operator=(ApiClient&&) noexcept;

    /**
     * @brief Get list of all devices
     * @return List of device information or error
     */
    Expected<std::vector<DeviceInfo>> get_devices();

    /**
     * @brief Get detailed device status
     * @param device_id Device ID to query
     * @return Device status information or error
     */
    Expected<DeviceStatus> get_device_status(const std::string& device_id);

    /**
     * @brief Send control command to device
     * @param device_id Device ID to control
     * @param command Command name
     * @param parameters Command parameters as JSON string
     * @return Success or error
     */
    Expected<void> send_command(const std::string& device_id, 
                                const std::string& command,
                                const std::string& parameters);

    /**
     * @brief Get device profile information
     * @param device_id Device ID to query
     * @return Device profile as JSON string or error
     */
    Expected<std::string> get_device_profile(const std::string& device_id);

    /**
     * @brief Set request timeout
     * @param timeout_ms Timeout in milliseconds
     */
    void set_timeout(long timeout_ms) { timeout_ms_ = timeout_ms; }

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
    
    std::shared_ptr<Auth> auth_;
    long timeout_ms_{30000}; // 30 second default timeout
    
    /// Internal HTTP GET request
    Expected<std::string> http_get(const std::string& url, 
                                    const std::map<std::string, std::string>& headers);
    
    /// Internal HTTP POST request
    Expected<std::string> http_post(const std::string& url,
                                     const std::string& body,
                                     const std::map<std::string, std::string>& headers);
};

} // namespace thinq_proxy
