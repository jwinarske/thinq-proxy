// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/refrigerator.hpp"

namespace thinq_proxy {

Expected<void> Refrigerator::set_fridge_temperature(float temp) {
    std::map<std::string, std::string> params = {
        {"temperature", std::to_string(static_cast<int>(temp))},
        {"zone", "fridge"}
    };
    
    auto result = send_command("setTemperature", params);
    if (result) {
        fridge_temp_ = temp;
    }
    return result;
}

Expected<void> Refrigerator::set_freezer_temperature(float temp) {
    std::map<std::string, std::string> params = {
        {"temperature", std::to_string(static_cast<int>(temp))},
        {"zone", "freezer"}
    };
    
    auto result = send_command("setTemperature", params);
    if (result) {
        freezer_temp_ = temp;
    }
    return result;
}

void Refrigerator::parse_status(const std::string& json_data) {
    // TODO: Parse JSON using glaze
    // Extract: fridge_temp_, freezer_temp_, door_open_
}

} // namespace thinq_proxy
