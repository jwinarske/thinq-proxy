#pragma once

#include <string>
#include <cstdint>

namespace thinq_proxy {

enum class Country {
    US,
    KR,
    EU,
    UK,
    JP,
    CN
};

inline std::string country_to_string(Country country) {
    switch (country) {
        case Country::US: return "US";
        case Country::KR: return "KR";
        case Country::EU: return "EU";
        case Country::UK: return "UK";
        case Country::JP: return "JP";
        case Country::CN: return "CN";
        default: return "US";
    }
}

enum class DeviceType {
    AirConditioner,
    Washer,
    Dryer,
    Refrigerator,
    AirPurifier,
    Dishwasher,
    Unknown
};

inline std::string device_type_to_string(DeviceType type) {
    switch (type) {
        case DeviceType::AirConditioner: return "AIR_CONDITIONER";
        case DeviceType::Washer: return "WASHER";
        case DeviceType::Dryer: return "DRYER";
        case DeviceType::Refrigerator: return "REFRIGERATOR";
        case DeviceType::AirPurifier: return "AIR_PURIFIER";
        case DeviceType::Dishwasher: return "DISHWASHER";
        default: return "UNKNOWN";
    }
}

inline DeviceType string_to_device_type(const std::string& str) {
    if (str == "AIR_CONDITIONER") return DeviceType::AirConditioner;
    if (str == "WASHER") return DeviceType::Washer;
    if (str == "DRYER") return DeviceType::Dryer;
    if (str == "REFRIGERATOR") return DeviceType::Refrigerator;
    if (str == "AIR_PURIFIER") return DeviceType::AirPurifier;
    if (str == "DISHWASHER") return DeviceType::Dishwasher;
    return DeviceType::Unknown;
}

struct DeviceStatus {
    bool online{false};
    std::string raw_data;
};

} // namespace thinq_proxy
