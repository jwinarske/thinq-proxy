// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

/**
 * @brief Air Conditioner device implementation
 * 
 * Maps ThinQ Air Conditioner to Matter Thermostat
 */
class AirConditioner : public Device {
public:
    using Device::Device;
    
    /// Get current temperature (°C)
    float get_current_temperature() const { return current_temp_; }
    
    /// Get target temperature (°C)
    float get_target_temperature() const { return target_temp_; }
    
    /// Set target temperature
    Expected<void> set_target_temperature(float temp);
    
    /// Get power state
    bool is_powered_on() const { return power_on_; }
    
    /// Set power state
    Expected<void> set_power(bool on);
    
    /// Get operation mode
    enum class Mode { Cool, Heat, Dry, Fan, Auto };
    Mode get_mode() const { return mode_; }
    
    /// Set operation mode
    Expected<void> set_mode(Mode mode);

protected:
    void parse_status(const std::string& json_data) override;

private:
    float current_temp_{25.0f};
    float target_temp_{24.0f};
    bool power_on_{false};
    Mode mode_{Mode::Cool};
};

} // namespace thinq_proxy
