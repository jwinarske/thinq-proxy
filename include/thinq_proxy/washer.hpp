// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "thinq_proxy/device.hpp"

namespace thinq_proxy {

/**
 * @brief Washer device implementation
 * 
 * Maps ThinQ Washer to Matter Laundry Washer
 */
class Washer : public Device {
public:
    using Device::Device;
    
    /// Washer state
    enum class State { Idle, Running, Paused, Complete, Error };
    
    /// Get current state
    State get_state() const { return state_; }
    
    /// Get remaining time (seconds)
    int get_remaining_time() const { return remaining_time_; }
    
    /// Start washing cycle
    Expected<void> start();
    
    /// Pause washing cycle
    Expected<void> pause();
    
    /// Stop washing cycle
    Expected<void> stop();

protected:
    void parse_status(const std::string& json_data) override;

private:
    State state_{State::Idle};
    int remaining_time_{0};
};

} // namespace thinq_proxy
