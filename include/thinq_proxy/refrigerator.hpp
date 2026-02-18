// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

/**
 * @brief Refrigerator device implementation
 * 
 * Maps ThinQ Refrigerator to Matter Refrigerator
 */
class Refrigerator : public Device {
public:
    using Device::Device;
    
    /// Get fridge temperature (°C)
    float get_fridge_temperature() const { return fridge_temp_; }
    
    /// Get freezer temperature (°C)
    float get_freezer_temperature() const { return freezer_temp_; }
    
    /// Set fridge temperature
    Expected<void> set_fridge_temperature(float temp);
    
    /// Set freezer temperature
    Expected<void> set_freezer_temperature(float temp);
    
    /// Check if door is open
    bool is_door_open() const { return door_open_; }

protected:
    void parse_status(const std::string& json_data) override;

private:
    float fridge_temp_{4.0f};
    float freezer_temp_{-18.0f};
    bool door_open_{false};
};

} // namespace thinq_proxy
