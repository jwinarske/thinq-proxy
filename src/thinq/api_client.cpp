// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/api_client.hpp"
#include <curl/curl.h>
#include <format>
#include <stdexcept>
#include <mutex>

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
    
    std::map<std::string, std::string> headers = {
        {"Authorization", auth_->get_auth_header()},
        {"x-client-id", auth_->client_id()},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_get(endpoints.device_url, headers);
    if (!response) {
        return std::unexpected(response.error());
    }
    
    // TODO: Parse JSON response using glaze
    // For now, return empty list as placeholder
    std::vector<DeviceInfo> devices;
    
    // Placeholder parsing - in real implementation, use glaze to parse JSON
    // Example response format:
    // {"result": {"devices": [{"deviceId": "...", "alias": "...", ...}]}}
    
    return make_success(std::move(devices));
}

Expected<DeviceStatus> ApiClient::get_device_status(const std::string& device_id) {
    auto endpoints = get_endpoints(auth_->country());
    std::string url = std::format("{}/{}", endpoints.device_url, device_id);
    
    std::map<std::string, std::string> headers = {
        {"Authorization", auth_->get_auth_header()},
        {"x-client-id", auth_->client_id()},
        {"Content-Type", "application/json"}
    };
    
    auto response = http_get(url, headers);
    if (!response) {
        return std::unexpected(response.error());
    }
    
    // TODO: Parse JSON response using glaze
    DeviceStatus status;
    status.device_id = device_id;
    status.online = true;
    
    return make_success(std::move(status));
}

Expected<void> ApiClient::send_command(const std::string& device_id,
                                        const std::string& command,
                                        const std::string& parameters) {
    auto endpoints = get_endpoints(auth_->country());
    std::string url = std::format("{}/{}/control", endpoints.control_url, device_id);
    
    std::map<std::string, std::string> headers = {
        {"Authorization", auth_->get_auth_header()},
        {"x-client-id", auth_->client_id()},
        {"Content-Type", "application/json"}
    };
    
    // Build request body
    std::string body = std::format(
        R"({{"command": "{}", "parameters": {}}})",
        command, parameters
    );
    
    auto response = http_post(url, body, headers);
    if (!response) {
        return std::unexpected(response.error());
    }
    
    return {};
}

Expected<std::string> ApiClient::get_device_profile(const std::string& device_id) {
    auto endpoints = get_endpoints(auth_->country());
    std::string url = std::format("{}/{}/profile", endpoints.device_url, device_id);
    
    std::map<std::string, std::string> headers = {
        {"Authorization", auth_->get_auth_header()},
        {"x-client-id", auth_->client_id()},
        {"Content-Type", "application/json"}
    };
    
    return http_get(url, headers);
}

} // namespace thinq_proxy
