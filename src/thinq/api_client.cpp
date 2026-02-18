#include "thinq_proxy/api_client.hpp"
#include <curl/curl.h>
#include <format>
#include <stdexcept>

namespace thinq_proxy {

// Callback for libcurl to write response data
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total_size);
    return total_size;
}

class ApiClient::Impl {
public:
    explicit Impl(std::shared_ptr<Auth> auth)
        : auth_(std::move(auth))
        , curl_(nullptr) {
    }
    
    ~Impl() {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
        curl_global_cleanup();
    }
    
    Result<void> initialize() {
        CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (res != CURLE_OK) {
            return std::unexpected(make_error_code(ErrorCode::NetworkError));
        }
        
        curl_ = curl_easy_init();
        if (!curl_) {
            return std::unexpected(make_error_code(ErrorCode::NetworkError));
        }
        
        auto valid = auth_->validate();
        if (!valid) {
            return std::unexpected(valid.error());
        }
        
        return {};
    }
    
    Result<std::vector<DevicePtr>> get_devices() {
        if (!curl_) {
            return std::unexpected(make_error_code(ErrorCode::NetworkError));
        }
        
        // ThinQ Connect API endpoint (example)
        std::string url = std::format("https://aic-service.lgthinq.com/{}/service/devices",
                                     country_to_string(auth_->get_country()));
        
        std::string response;
        auto result = perform_request(url, response);
        if (!result) {
            return std::unexpected(result.error());
        }
        
        // Parse response and create device objects
        // For now, return empty vector as placeholder
        std::vector<DevicePtr> devices;
        
        // TODO: Parse JSON response using glaze and create device objects
        // This is a simplified implementation
        
        return devices;
    }
    
    Result<std::string> get_device_status(const std::string& device_id) {
        if (!curl_) {
            return std::unexpected(make_error_code(ErrorCode::NetworkError));
        }
        
        std::string url = std::format("https://aic-service.lgthinq.com/{}/service/devices/{}/status",
                                     country_to_string(auth_->get_country()),
                                     device_id);
        
        std::string response;
        auto result = perform_request(url, response);
        if (!result) {
            return std::unexpected(result.error());
        }
        
        return response;
    }
    
    Result<void> send_device_command(const std::string& device_id,
                                     const std::string& command,
                                     const std::string& parameters) {
        if (!curl_) {
            return std::unexpected(make_error_code(ErrorCode::NetworkError));
        }
        
        std::string url = std::format("https://aic-service.lgthinq.com/{}/service/devices/{}/control",
                                     country_to_string(auth_->get_country()),
                                     device_id);
        
        std::string response;
        auto result = perform_request(url, response, "POST", parameters);
        if (!result) {
            return std::unexpected(result.error());
        }
        
        return {};
    }

private:
    Result<void> perform_request(const std::string& url, 
                                 std::string& response,
                                 const std::string& method = "GET",
                                 const std::string& body = "") {
        curl_easy_reset(curl_);
        
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
        
        // Set headers
        struct curl_slist* headers = nullptr;
        auto auth_header = std::format("Authorization: {}", auth_->get_auth_header());
        headers = curl_slist_append(headers, auth_header.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, std::format("x-client-id: {}", auth_->get_client_id()).c_str());
        
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
        
        if (method == "POST") {
            curl_easy_setopt(curl_, CURLOPT_POST, 1L);
            if (!body.empty()) {
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
            }
        }
        
        CURLcode res = curl_easy_perform(curl_);
        curl_slist_free_all(headers);
        
        if (res != CURLE_OK) {
            return std::unexpected(make_error_code(ErrorCode::NetworkError));
        }
        
        long http_code = 0;
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
        
        if (http_code < 200 || http_code >= 300) {
            if (http_code == 401 || http_code == 403) {
                return std::unexpected(make_error_code(ErrorCode::AuthenticationFailed));
            }
            return std::unexpected(make_error_code(ErrorCode::InvalidResponse));
        }
        
        return {};
    }
    
    std::shared_ptr<Auth> auth_;
    CURL* curl_;
};

ApiClient::ApiClient(std::shared_ptr<Auth> auth)
    : impl_(std::make_unique<Impl>(std::move(auth))) {
}

ApiClient::~ApiClient() = default;

Result<void> ApiClient::initialize() {
    return impl_->initialize();
}

Result<std::vector<DevicePtr>> ApiClient::get_devices() {
    return impl_->get_devices();
}

Result<std::string> ApiClient::get_device_status(const std::string& device_id) {
    return impl_->get_device_status(device_id);
}

Result<void> ApiClient::send_device_command(const std::string& device_id,
                                           const std::string& command,
                                           const std::string& parameters) {
    return impl_->send_device_command(device_id, command, parameters);
}

} // namespace thinq_proxy
