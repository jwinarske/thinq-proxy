#include "thinq_proxy/devices/washer.hpp"

namespace thinq_proxy {

Washer::Washer(std::string device_id, std::string alias)
    : Device(std::move(device_id), std::move(alias), DeviceType::Washer) {
}

Result<void> Washer::update_status(const std::string& status_data) {
    status_.online = true;
    status_.raw_data = status_data;
    
    // TODO: Parse JSON to extract:
    // - state_
    // - remaining_time_
    
    return {};
}

Result<void> Washer::send_command(const std::string& command, const std::string& parameters) {
    if (command.empty()) {
        return std::unexpected(make_error_code(ErrorCode::DeviceCommandFailed));
    }
    
    // Commands: "start", "pause", "stop"
    
    return {};
}

} // namespace thinq_proxy
