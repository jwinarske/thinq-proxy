// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/api_client.hpp"
#include <curl/curl.h>
#include "glaze/glaze.hpp"

#include <iostream>
#include <array>
#include <random>
#include <chrono>

namespace thinq_proxy {

struct ThinQDeviceInfoPayload {
    std::string alias;
    std::string modelName;
    std::string deviceType;
    std::string deviceTypeCode;
    bool reportable{true};
};

struct ThinQDevicePayload {
    std::string deviceId;
    std::string alias;
    std::string modelName;
    std::string deviceType;
    std::string deviceTypeCode;
    bool reportable{true};
    ThinQDeviceInfoPayload deviceInfo;
};

struct ThinQDevicesResponse {
    std::vector<ThinQDevicePayload> response;
};

struct ThinQLegacyDevicesResult {
    std::vector<ThinQDevicePayload> devices;
};

struct ThinQLegacyDevicesResponse {
    ThinQLegacyDevicesResult result;
};

struct ThinQGenericResponse {
    glz::generic response;
};

namespace {

constexpr std::string_view kThinQApiKey = "v6GFvkweNo7DK7yD3ylIZ9w52aKBU0eJ7wLXkSR3";

std::string generate_message_id() {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::array<unsigned char, 16> bytes{};
    for (auto& byte : bytes) {
        byte = static_cast<unsigned char>(rng() & 0xFF);
    }

    bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0F) | 0x40); // UUIDv4 version
    bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3F) | 0x80); // RFC variant

    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    out.reserve(22);

    uint32_t buffer = 0;
    int bits = 0;
    for (const auto b : bytes) {
        buffer = (buffer << 8) | b;
        bits += 8;
        while (bits >= 6) {
            bits -= 6;
            out.push_back(alphabet[(buffer >> bits) & 0x3F]);
        }
    }
    if (bits > 0) {
        out.push_back(alphabet[(buffer << (6 - bits)) & 0x3F]);
    }

    if (out.size() > 22) {
        out.resize(22);
    }
    return out;
}

std::map<std::string, std::string> make_common_headers(const Auth& auth) {
    return {
        {"Authorization", auth.get_auth_header()},
        {"x-message-id", generate_message_id()},
        {"x-country", std::string(to_string(auth.country()))},
        {"x-client-id", auth.client_id()},
        {"x-api-key", std::string(kThinQApiKey)},
        {"Content-Type", "application/json"}
    };
}

std::string first_non_empty(std::initializer_list<std::string_view> values) {
    for (const auto value : values) {
        if (!value.empty()) {
            return std::string(value);
        }
    }
    return {};
}

DeviceType parse_device_type(std::string_view raw_type) {
    if (raw_type.empty()) {
        return DeviceType::Unknown;
    }

    std::string normalized;
    normalized.reserve(raw_type.size());
    for (const auto c : raw_type) {
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }

    if (normalized.starts_with("DEVICE_")) {
        normalized.erase(0, 7);
    }

    if (normalized.find("AIR_CONDITIONER") != std::string::npos) return DeviceType::AirConditioner;
    if (normalized.find("WASHER") != std::string::npos) return DeviceType::Washer;
    if (normalized.find("DRYER") != std::string::npos) return DeviceType::Dryer;
    if (normalized.find("REFRIGERATOR") != std::string::npos) return DeviceType::Refrigerator;
    if (normalized.find("AIR_PURIFIER") != std::string::npos) return DeviceType::AirPurifier;
    if (normalized.find("DISHWASHER") != std::string::npos) return DeviceType::Dishwasher;
    if (normalized.find("OVEN") != std::string::npos) return DeviceType::Oven;
    if (normalized.find("ROBOT_CLEANER") != std::string::npos) return DeviceType::RobotCleaner;
    if (normalized.find("WATER_PURIFIER") != std::string::npos) return DeviceType::WaterPurifier;

    return DeviceType::Unknown;
}

DeviceType extract_device_type(const glz::generic& response_payload) {
    const auto* payload_obj = response_payload.get_if<glz::generic::object_t>();
    if (!payload_obj) {
        return DeviceType::Unknown;
    }

    if (const auto it = payload_obj->find("deviceType"); it != payload_obj->end()) {
        if (const auto* value = it->second.get_if<std::string>()) {
            return parse_device_type(*value);
        }
    }

    if (const auto it = payload_obj->find("deviceInfo"); it != payload_obj->end()) {
        if (const auto* info_obj = it->second.get_if<glz::generic::object_t>()) {
            if (const auto type_it = info_obj->find("deviceType"); type_it != info_obj->end()) {
                if (const auto* value = type_it->second.get_if<std::string>()) {
                    return parse_device_type(*value);
                }
            }
        }
    }

    return DeviceType::Unknown;
}

std::vector<DeviceInfo> to_device_infos(const std::vector<ThinQDevicePayload>& payloads) {
    std::vector<DeviceInfo> devices;
    devices.reserve(payloads.size());

    for (const auto& item : payloads) {
        const auto device_id = first_non_empty({item.deviceId});
        if (device_id.empty()) {
            continue;
        }

        const auto alias = first_non_empty({item.deviceInfo.alias, item.alias});
        const auto model_name = first_non_empty({item.deviceInfo.modelName, item.modelName});
        const auto type_text = first_non_empty({
            item.deviceInfo.deviceType,
            item.deviceType,
            item.deviceInfo.deviceTypeCode,
            item.deviceTypeCode
        });

        DeviceInfo info;
        info.device_id = device_id;
        info.alias = alias;
        info.model_name = model_name;
        info.type = parse_device_type(type_text);
        info.device_name = !alias.empty() ? alias : (!model_name.empty() ? model_name : device_id);
        info.reportable = item.deviceInfo.reportable || item.reportable;

        devices.push_back(std::move(info));
    }

    return devices;
}

void print_response_json(const std::string& response_body, std::ostream& out = std::cout) {
    if (response_body.empty()) {
        out << "ThinQ response: <empty>\n";
        return;
    }

    std::string pretty;
    glz::prettify_json(response_body, pretty);

    if (pretty.empty()) {
        out << "ThinQ response:\n" << response_body << '\n';
        return;
    }

    out << "ThinQ response:\n" << pretty << '\n';
}

} // namespace

} // namespace thinq_proxy

namespace glz {

template <>
struct meta<thinq_proxy::ThinQDeviceInfoPayload> {
    using T = thinq_proxy::ThinQDeviceInfoPayload;
    static constexpr auto value = object(
        "alias", &T::alias,
        "modelName", &T::modelName,
        "deviceType", &T::deviceType,
        "deviceTypeCode", &T::deviceTypeCode,
        "reportable", &T::reportable
    );
};

template <>
struct meta<thinq_proxy::ThinQDevicePayload> {
    using T = thinq_proxy::ThinQDevicePayload;
    static constexpr auto value = object(
        "deviceId", &T::deviceId,
        "alias", &T::alias,
        "modelName", &T::modelName,
        "deviceType", &T::deviceType,
        "deviceTypeCode", &T::deviceTypeCode,
        "reportable", &T::reportable,
        "deviceInfo", &T::deviceInfo
    );
};

template <>
struct meta<thinq_proxy::ThinQDevicesResponse> {
    using T = thinq_proxy::ThinQDevicesResponse;
    static constexpr auto value = object(
        "response", &T::response
    );
};

template <>
struct meta<thinq_proxy::ThinQLegacyDevicesResult> {
    using T = thinq_proxy::ThinQLegacyDevicesResult;
    static constexpr auto value = object(
        "devices", &T::devices
    );
};

template <>
struct meta<thinq_proxy::ThinQLegacyDevicesResponse> {
    using T = thinq_proxy::ThinQLegacyDevicesResponse;
    static constexpr auto value = object(
        "result", &T::result
    );
};

template <>
struct meta<thinq_proxy::ThinQGenericResponse> {
    using T = thinq_proxy::ThinQGenericResponse;
    static constexpr auto value = object(
        "response", &T::response
    );
};

} // namespace glz

namespace thinq_proxy {

// PIMPL implementation
struct ApiClient::Impl {
    CURL* curl{nullptr};
    
    // Static initialization flag for curl_global_init
    static bool curl_initialized;
    static std::mutex curl_init_mutex;
    
    static void ensure_curl_init() {
        std::lock_guard<std::mutex> lock(curl_init_mutex);
        if (!curl_initialized) {
            curl_global_init(CURL_GLOBAL_DEFAULT);
            curl_initialized = true;
        }
    }
    
    Impl() {
        ensure_curl_init();
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize curl");
        }
    }
    
    ~Impl() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        // Note: curl_global_cleanup() should be called at application exit,
        // not here, as other ApiClient instances may still be using curl
    }
    
    // Callback for curl write data
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t total_size = size * nmemb;
        auto* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), total_size);
        return total_size;
    }
};

// Static member initialization
bool ApiClient::Impl::curl_initialized = false;
std::mutex ApiClient::Impl::curl_init_mutex;

ApiClient::ApiClient(std::shared_ptr<Auth> auth)
    : pimpl_(std::make_unique<Impl>())
    , auth_(std::move(auth)) {
    
    if (!auth_ || !auth_->is_valid()) {
        throw std::runtime_error("Invalid authentication credentials");
    }
}

ApiClient::~ApiClient() = default;

ApiClient::ApiClient(ApiClient&&) noexcept = default;
ApiClient& ApiClient::operator=(ApiClient&&) noexcept = default;

Expected<std::string> ApiClient::http_get(const std::string& url,
                                           const std::map<std::string, std::string>& headers) {
    if (!pimpl_->curl) {
        return make_error<std::string>(ErrorCode::NetworkError, "CURL not initialized");
    }
    
    std::string response;
    struct curl_slist* header_list = nullptr;
    
    // Set URL
    curl_easy_setopt(pimpl_->curl, CURLOPT_URL, url.c_str());
    
    // Set headers
    for (const auto& [key, value] : headers) {
        std::string header = std::format("{}: {}", key, value);
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(pimpl_->curl, CURLOPT_HTTPHEADER, header_list);
    
    // Set write callback
    curl_easy_setopt(pimpl_->curl, CURLOPT_WRITEFUNCTION, Impl::write_callback);
    curl_easy_setopt(pimpl_->curl, CURLOPT_WRITEDATA, &response);
    
    // Set timeout
    curl_easy_setopt(pimpl_->curl, CURLOPT_TIMEOUT_MS, timeout_ms_);
    
    // Perform request
    CURLcode res = curl_easy_perform(pimpl_->curl);
    
    // Clean up header list
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    
    if (res != CURLE_OK) {
        return make_error<std::string>(
            ErrorCode::NetworkError,
            std::format("HTTP GET failed: {}", curl_easy_strerror(res))
        );
    }
    
    // Check HTTP status code
    long http_code = 0;
    curl_easy_getinfo(pimpl_->curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code >= 400) {
        ErrorCode error_code = ErrorCode::ApiError;
        if (http_code == 401 || http_code == 403) {
            error_code = ErrorCode::AuthenticationFailed;
        } else if (http_code == 404) {
            error_code = ErrorCode::DeviceNotFound;
        }
        return make_error<std::string>(error_code, 
                                       std::format("HTTP error: {}", http_code),
                                       static_cast<int>(http_code));
    }
    
    return make_success(std::move(response));
}

Expected<std::string> ApiClient::http_post(const std::string& url,
                                            const std::string& body,
                                            const std::map<std::string, std::string>& headers) {
    if (!pimpl_->curl) {
        return make_error<std::string>(ErrorCode::NetworkError, "CURL not initialized");
    }
    
    std::string response;
    struct curl_slist* header_list = nullptr;
    
    // Set URL
    curl_easy_setopt(pimpl_->curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(pimpl_->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(pimpl_->curl, CURLOPT_POSTFIELDS, body.c_str());
    
    // Set headers
    for (const auto& [key, value] : headers) {
        std::string header = std::format("{}: {}", key, value);
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(pimpl_->curl, CURLOPT_HTTPHEADER, header_list);
    
    // Set write callback
    curl_easy_setopt(pimpl_->curl, CURLOPT_WRITEFUNCTION, Impl::write_callback);
    curl_easy_setopt(pimpl_->curl, CURLOPT_WRITEDATA, &response);
    
    // Set timeout
    curl_easy_setopt(pimpl_->curl, CURLOPT_TIMEOUT_MS, timeout_ms_);
    
    // Perform request
    CURLcode res = curl_easy_perform(pimpl_->curl);
    
    // Clean up header list
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    
    if (res != CURLE_OK) {
        return make_error<std::string>(
            ErrorCode::NetworkError,
            std::format("HTTP POST failed: {}", curl_easy_strerror(res))
        );
    }
    
    // Check HTTP status code
    long http_code = 0;
    curl_easy_getinfo(pimpl_->curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code >= 400) {
        ErrorCode error_code = ErrorCode::ApiError;
        if (http_code == 401 || http_code == 403) {
            error_code = ErrorCode::AuthenticationFailed;
        }
        return make_error<std::string>(error_code,
                                       std::format("HTTP error: {}", http_code),
                                       static_cast<int>(http_code));
    }
    
    return make_success(std::move(response));
}

Expected<std::vector<DeviceInfo>> ApiClient::get_devices() {
    auto endpoints = get_endpoints(auth_->country());
    auto headers = make_common_headers(*auth_);
    
    auto response = http_get(endpoints.device_url, headers);
    if (!response) {
        return std::unexpected(response.error());
    }

    print_response_json(response.value());

    constexpr auto relaxed_opts = glz::opts{.error_on_unknown_keys = false};

    ThinQDevicesResponse parsed;
    if (const auto ec = glz::read<relaxed_opts>(parsed, response.value()); !ec) {
        return make_success(to_device_infos(parsed.response));
    }

    ThinQLegacyDevicesResponse legacy;
    if (const auto ec = glz::read<relaxed_opts>(legacy, response.value()); !ec) {
        return make_success(to_device_infos(legacy.result.devices));
    }

    return make_error<std::vector<DeviceInfo>>(
        ErrorCode::ParseError,
        "Failed to parse device list response from ThinQ API"
    );
}

Expected<DeviceStatus> ApiClient::get_device_status(const std::string& device_id) {
    auto endpoints = get_endpoints(auth_->country());
    std::string url = std::format("{}/{}/state", endpoints.device_url, device_id);
    auto headers = make_common_headers(*auth_);
    
    auto response = http_get(url, headers);
    if (!response) {
        return std::unexpected(response.error());
    }

    print_response_json(response.value());

    constexpr auto relaxed_opts = glz::opts{.error_on_unknown_keys = false};
    ThinQGenericResponse parsed;
    if (const auto ec = glz::read<relaxed_opts>(parsed, response.value()); ec) {
        return make_error<DeviceStatus>(
            ErrorCode::ParseError,
            std::format("Failed to parse device status response: {}", glz::format_error(ec, response.value()))
        );
    }

    DeviceStatus status;
    status.device_id = device_id;
    status.type = extract_device_type(parsed.response);
    status.online = true;
    status.last_update_ms = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count());

    if (const auto* payload_obj = parsed.response.get_if<glz::generic::object_t>()) {
        if (const auto online_it = payload_obj->find("online"); online_it != payload_obj->end()) {
            if (const auto* online = online_it->second.get_if<bool>()) {
                status.online = *online;
            }
        }
    }
    
    return make_success(std::move(status));
}

Expected<void> ApiClient::send_command(const std::string& device_id,
                                        const std::string& command,
                                        const std::string& parameters) {
    auto endpoints = get_endpoints(auth_->country());
    std::string url = std::format("{}/{}/control", endpoints.control_url, device_id);
    auto headers = make_common_headers(*auth_);
    
    std::string command_payload = parameters.empty() ? "{}" : parameters;
    std::string body = command.empty()
        ? command_payload
        : std::format(R"({{"{}": {}}})", command, command_payload);

    glz::generic request_json;
    if (const auto ec = glz::read_json(request_json, body); ec) {
        return make_error<void>(
            ErrorCode::InvalidRequest,
            std::format("Invalid control payload JSON: {}", glz::format_error(ec, body))
        );
    }
    
    auto response = http_post(url, body, headers);
    if (!response) {
        return std::unexpected(response.error());
    }

    print_response_json(response.value());

    constexpr auto relaxed_opts = glz::opts{.error_on_unknown_keys = false};
    ThinQGenericResponse parsed;
    if (const auto ec = glz::read<relaxed_opts>(parsed, response.value()); ec) {
        return make_error<void>(
            ErrorCode::ParseError,
            std::format("Failed to parse control response: {}", glz::format_error(ec, response.value()))
        );
    }
    
    return {};
}

Expected<std::string> ApiClient::get_device_profile(const std::string& device_id) {
    auto endpoints = get_endpoints(auth_->country());
    std::string url = std::format("{}/{}/profile", endpoints.device_url, device_id);
    auto headers = make_common_headers(*auth_);

    auto response = http_get(url, headers);
    if (!response) {
        return std::unexpected(response.error());
    }

    print_response_json(response.value());

    constexpr auto relaxed_opts = glz::opts{.error_on_unknown_keys = false};
    ThinQGenericResponse parsed;
    if (const auto ec = glz::read<relaxed_opts>(parsed, response.value()); ec) {
        return make_error<std::string>(
            ErrorCode::ParseError,
            std::format("Failed to parse device profile response: {}", glz::format_error(ec, response.value()))
        );
    }

    const auto profile_json = parsed.response.dump();
    if (!profile_json) {
        return make_error<std::string>(
            ErrorCode::ParseError,
            std::format("Failed to serialize parsed device profile response: {}", glz::format_error(profile_json))
        );
    }

    return make_success(profile_json.value());
}

} // namespace thinq_proxy
