// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include <expected>
#include <string>
#include <system_error>
#include <format>

namespace thinq_proxy {

/// Error codes for ThinQ operations
enum class ErrorCode {
    Success = 0,
    NetworkError,
    AuthenticationFailed,
    InvalidToken,
    DeviceNotFound,
    InvalidRequest,
    ApiError,
    ParseError,
    ConfigError,
    MatterError,
    Timeout,
    Unknown
};

/// Error information
struct Error {
    ErrorCode code;
    std::string message;
    int http_status{0};

    Error(ErrorCode c, std::string msg, int status = 0)
        : code(c), message(std::move(msg)), http_status(status) {}

    std::string to_string() const {
        if (http_status > 0) {
            return std::format("Error {}: {} (HTTP {})", 
                             static_cast<int>(code), message, http_status);
        }
        return std::format("Error {}: {}", static_cast<int>(code), message);
    }
};

/// Expected type for operations that can fail
template<typename T>
using Expected = std::expected<T, Error>;

/// Helper to create error result
template<typename T>
inline Expected<T> make_error(ErrorCode code, std::string message, int status = 0) {
    return std::unexpected(Error(code, std::move(message), status));
}

/// Helper to create success result
template<typename T>
inline Expected<T> make_success(T&& value) {
    return Expected<T>(std::forward<T>(value));
}

} // namespace thinq_proxy
