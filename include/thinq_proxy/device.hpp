#pragma once

#include "error.hpp"
#include "types.hpp"
#include <string>
#include <memory>

namespace thinq_proxy {

/**
 * @brief Base class for all ThinQ devices
 */
class Device {
public:
    Device(std::string device_id, std::string alias, DeviceType type);
    virtual ~Device() = default;
    
    /**
     * @brief Get device ID
     */
    const std::string& get_id() const { return device_id_; }
    
    /**
     * @brief Get device alias/name
     */
    const std::string& get_alias() const { return alias_; }
    
    /**
     * @brief Get device type
     */
    DeviceType get_type() const { return type_; }
    
    /**
     * @brief Update device status from API response
     * @param status_data Raw JSON status data
     */
    virtual Result<void> update_status(const std::string& status_data) = 0;
    
    /**
     * @brief Get current device status
     */
    const DeviceStatus& get_status() const { return status_; }
    
    /**
     * @brief Send command to device
     * @param command Command name
     * @param parameters Command parameters as JSON string
     */
    virtual Result<void> send_command(const std::string& command, 
                                     const std::string& parameters) = 0;

protected:
    std::string device_id_;
    std::string alias_;
    DeviceType type_;
    DeviceStatus status_;
};

using DevicePtr = std::shared_ptr<Device>;

} // namespace thinq_proxy
