#pragma once

#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

/**
 * @brief Air Conditioner device implementation
 * Maps to Matter Thermostat
 */
class AirConditioner : public Device {
public:
    AirConditioner(std::string device_id, std::string alias);
    
    Result<void> update_status(const std::string& status_data) override;
    Result<void> send_command(const std::string& command, const std::string& parameters) override;
    
    // AC-specific properties
    bool is_on() const { return is_on_; }
    int get_target_temperature() const { return target_temp_; }
    int get_current_temperature() const { return current_temp_; }
    
    enum class Mode {
        Cool,
        Heat,
        Dry,
        Fan,
        Auto
    };
    
    Mode get_mode() const { return mode_; }

private:
    bool is_on_{false};
    int target_temp_{24};
    int current_temp_{22};
    Mode mode_{Mode::Cool};
};

} // namespace thinq_proxy
