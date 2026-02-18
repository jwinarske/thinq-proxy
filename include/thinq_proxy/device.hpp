// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "types.hpp"
#include "error.hpp"
#include <string>
#include <memory>
#include <map>

namespace thinq_proxy {

class ApiClient;

/**
 * @brief Base class for ThinQ devices
 * 
 * Represents a generic ThinQ device with common functionality.
 * Specific device types (AC, Washer, etc.) inherit from this class.
 */
class Device {
public:
    /**
     * @brief Construct device from device info
     * @param info Device information from API
     * @param client API client for device communication
     */
    Device(DeviceInfo info, std::shared_ptr<ApiClient> client);
    
    virtual ~Device() = default;

    /// Get device ID
    const std::string& device_id() const { return info_.device_id; }
    
    /// Get device name
    const std::string& device_name() const { return info_.device_name; }
    
    /// Get device type
    DeviceType type() const { return info_.type; }
    
    /// Get device info
    const DeviceInfo& info() const { return info_; }
    
    /**
     * @brief Update device status from API
     * @return Success or error
     */
    virtual Expected<void> update_status();
    
    /**
     * @brief Get current device status
     * @return Device status
     */
    virtual const DeviceStatus& status() const { return status_; }
    
    /**
     * @brief Send command to device
     * @param command Command name
     * @param params Command parameters
     * @return Success or error
     */
    virtual Expected<void> send_command(const std::string& command,
                                        const std::map<std::string, std::string>& params);

protected:
    DeviceInfo info_;
    DeviceStatus status_;
    std::shared_ptr<ApiClient> client_;
    
    /// Parse device-specific status (override in subclasses)
    virtual void parse_status(const std::string& json_data) {}
};

/**
 * @brief Factory function to create device from type
 * @param info Device information
 * @param client API client
 * @return Device instance
 */
std::shared_ptr<Device> create_device(const DeviceInfo& info, 
                                       std::shared_ptr<ApiClient> client);

} // namespace thinq_proxy
