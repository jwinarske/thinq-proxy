// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/air_conditioner.hpp"

namespace thinq_proxy {

Expected<void> AirConditioner::set_target_temperature(float temp) {
    std::map<std::string, std::string> params = {
        {"targetTemperature", std::to_string(static_cast<int>(temp))}
    };
    
    auto result = send_command("setTemperature", params);
    if (result) {
        target_temp_ = temp;
    }
    return result;
}

Expected<void> AirConditioner::set_power(bool on) {
    std::map<std::string, std::string> params = {
        {"power", on ? "on" : "off"}
    };
    
    auto result = send_command("setPower", params);
    if (result) {
        power_on_ = on;
    }
    return result;
}

Expected<void> AirConditioner::set_mode(Mode mode) {
    const char* mode_str = "cool";
    switch (mode) {
        case Mode::Cool: mode_str = "cool"; break;
        case Mode::Heat: mode_str = "heat"; break;
        case Mode::Dry: mode_str = "dry"; break;
        case Mode::Fan: mode_str = "fan"; break;
        case Mode::Auto: mode_str = "auto"; break;
    }
    
    std::map<std::string, std::string> params = {
        {"mode", mode_str}
    };
    
    auto result = send_command("setMode", params);
    if (result) {
        mode_ = mode;
    }
    return result;
}

void AirConditioner::parse_status([[maybe_unused]] const std::string& json_data) {
    // TODO: Parse JSON using glaze
    // Extract: current_temp_, target_temp_, power_on_, mode_
}

} // namespace thinq_proxy
