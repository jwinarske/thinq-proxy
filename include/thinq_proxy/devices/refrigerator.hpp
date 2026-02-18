#pragma once

#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

/**
 * @brief Refrigerator device implementation
 * Maps to Matter Refrigerator
 */
class Refrigerator : public Device {
public:
    Refrigerator(std::string device_id, std::string alias);
    
    Result<void> update_status(const std::string& status_data) override;
    Result<void> send_command(const std::string& command, const std::string& parameters) override;
    
    int get_fridge_temp() const { return fridge_temp_; }
    int get_freezer_temp() const { return freezer_temp_; }
    bool is_door_open() const { return door_open_; }

private:
    int fridge_temp_{4};   // Celsius
    int freezer_temp_{-18}; // Celsius
    bool door_open_{false};
};

} // namespace thinq_proxy
