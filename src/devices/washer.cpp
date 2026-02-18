// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/washer.hpp"

namespace thinq_proxy {

Expected<void> Washer::start() {
    std::map<std::string, std::string> params;
    auto result = send_command("start", params);
    if (result) {
        state_ = State::Running;
    }
    return result;
}

Expected<void> Washer::pause() {
    std::map<std::string, std::string> params;
    auto result = send_command("pause", params);
    if (result) {
        state_ = State::Paused;
    }
    return result;
}

Expected<void> Washer::stop() {
    std::map<std::string, std::string> params;
    auto result = send_command("stop", params);
    if (result) {
        state_ = State::Idle;
    }
    return result;
}

void Washer::parse_status(const std::string& json_data) {
    // TODO: Parse JSON using glaze
    // Extract: state_, remaining_time_
}

} // namespace thinq_proxy
