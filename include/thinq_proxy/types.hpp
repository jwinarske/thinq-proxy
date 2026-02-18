// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include <string>
#include <string_view>
#include <cstdint>

namespace thinq_proxy {

/// Country codes supported by ThinQ API
enum class CountryCode {
    US,  // United States
    KR,  // South Korea
    EU,  // European Union
    GB,  // United Kingdom
    CA,  // Canada
    AU,  // Australia
    JP,  // Japan
};

/// Convert CountryCode to string
constexpr std::string_view to_string(CountryCode code) {
    switch (code) {
        case CountryCode::US: return "US";
        case CountryCode::KR: return "KR";
        case CountryCode::EU: return "EU";
        case CountryCode::GB: return "GB";
        case CountryCode::CA: return "CA";
        case CountryCode::AU: return "AU";
        case CountryCode::JP: return "JP";
    }
    return "UNKNOWN";
}

/// Device types supported by ThinQ API
enum class DeviceType {
    AirConditioner,
    Washer,
    Dryer,
    Refrigerator,
    AirPurifier,
    Dishwasher,
    Oven,
    RobotCleaner,
    WaterPurifier,
    Unknown
};

/// Convert DeviceType to string
constexpr std::string_view to_string(DeviceType type) {
    switch (type) {
        case DeviceType::AirConditioner: return "AIR_CONDITIONER";
        case DeviceType::Washer: return "WASHER";
        case DeviceType::Dryer: return "DRYER";
        case DeviceType::Refrigerator: return "REFRIGERATOR";
        case DeviceType::AirPurifier: return "AIR_PURIFIER";
        case DeviceType::Dishwasher: return "DISHWASHER";
        case DeviceType::Oven: return "OVEN";
        case DeviceType::RobotCleaner: return "ROBOT_CLEANER";
        case DeviceType::WaterPurifier: return "WATER_PURIFIER";
        case DeviceType::Unknown: return "UNKNOWN";
    }
    return "UNKNOWN";
}

/// Device status information
struct DeviceStatus {
    std::string device_id;
    DeviceType type;
    bool online{false};
    uint64_t last_update_ms{0};
};

/// Device information
struct DeviceInfo {
    std::string device_id;
    std::string device_name;
    std::string model_name;
    DeviceType type;
    std::string alias;
    bool reportable{false};
};

/// API endpoint configuration
struct ApiEndpoints {
    std::string base_url;
    std::string oauth_url;
    std::string device_url;
    std::string control_url;
};

/// Get API endpoints for country
inline ApiEndpoints get_endpoints(CountryCode country) {
    // ThinQ Connect API endpoints
    const std::string base = "https://connect.lgthinq.com";
    return ApiEndpoints{
        .base_url = base,
        .oauth_url = base + "/oauth",
        .device_url = base + "/api/v1/devices",
        .control_url = base + "/api/v1/devices"
    };
}

} // namespace thinq_proxy
