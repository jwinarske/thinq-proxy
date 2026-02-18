// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/device.hpp"
#include "thinq_proxy/api_client.hpp"

namespace thinq_proxy {

Device::Device(DeviceInfo info, std::shared_ptr<ApiClient> client)
    : info_(std::move(info))
    , client_(std::move(client)) {
    status_.device_id = info_.device_id;
    status_.type = info_.type;
}

Expected<void> Device::update_status() {
    if (!client_) {
        return make_error<void>(ErrorCode::ApiError, "No API client available");
    }
    
    auto result = client_->get_device_status(info_.device_id);
    if (!result) {
        return std::unexpected(result.error());
    }
    
    status_ = result.value();
    return {};
}

Expected<void> Device::send_command(const std::string& command,
                                     const std::map<std::string, std::string>& params) {
    if (!client_) {
        return make_error<void>(ErrorCode::ApiError, "No API client available");
    }
    
    // Convert params map to JSON string
    // TODO: Use glaze for proper JSON serialization
    std::string json_params = "{}";
    if (!params.empty()) {
        json_params = "{";
        bool first = true;
        for (const auto& [key, value] : params) {
            if (!first) json_params += ",";
            json_params += std::format(R"("{}": "{}")", key, value);
            first = false;
        }
        json_params += "}";
    }
    
    return client_->send_command(info_.device_id, command, json_params);
}

std::shared_ptr<Device> create_device(const DeviceInfo& info,
                                       std::shared_ptr<ApiClient> client) {
    // For now, create base Device instances
    // In a full implementation, this would create device-specific subclasses
    return std::make_shared<Device>(info, client);
}

} // namespace thinq_proxy
