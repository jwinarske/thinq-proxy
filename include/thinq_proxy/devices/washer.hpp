#pragma once

#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

/**
 * @brief Washer device implementation
 * Maps to Matter Laundry Washer
 */
class Washer : public Device {
public:
    Washer(std::string device_id, std::string alias);
    
    Result<void> update_status(const std::string& status_data) override;
    Result<void> send_command(const std::string& command, const std::string& parameters) override;
    
    enum class State {
        Idle,
        Running,
        Paused,
        Complete
    };
    
    State get_state() const { return state_; }
    int get_remaining_time() const { return remaining_time_; }

private:
    State state_{State::Idle};
    int remaining_time_{0}; // in minutes
};

} // namespace thinq_proxy
