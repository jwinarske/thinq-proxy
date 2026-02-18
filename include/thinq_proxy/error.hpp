#pragma once

#include <expected>
#include <string>
#include <system_error>

namespace thinq_proxy {

enum class ErrorCode {
    Success = 0,
    NetworkError,
    AuthenticationFailed,
    InvalidResponse,
    DeviceNotFound,
    InvalidConfiguration,
    MatterInitFailed,
    DeviceCommandFailed,
    ParseError
};

class ErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override { return "thinq_proxy"; }
    
    std::string message(int ev) const override {
        switch (static_cast<ErrorCode>(ev)) {
            case ErrorCode::Success: return "Success";
            case ErrorCode::NetworkError: return "Network error";
            case ErrorCode::AuthenticationFailed: return "Authentication failed";
            case ErrorCode::InvalidResponse: return "Invalid response";
            case ErrorCode::DeviceNotFound: return "Device not found";
            case ErrorCode::InvalidConfiguration: return "Invalid configuration";
            case ErrorCode::MatterInitFailed: return "Matter initialization failed";
            case ErrorCode::DeviceCommandFailed: return "Device command failed";
            case ErrorCode::ParseError: return "Parse error";
            default: return "Unknown error";
        }
    }
};

inline const ErrorCategory& error_category() {
    static ErrorCategory instance;
    return instance;
}

inline std::error_code make_error_code(ErrorCode e) {
    return {static_cast<int>(e), error_category()};
}

template<typename T>
using Result = std::expected<T, std::error_code>;

} // namespace thinq_proxy

namespace std {
    template <>
    struct is_error_code_enum<thinq_proxy::ErrorCode> : true_type {};
}
