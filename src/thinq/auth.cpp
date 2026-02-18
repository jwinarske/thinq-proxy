// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/auth.hpp"
#include <format>

namespace thinq_proxy {

Auth::Auth(std::string token, std::string client_id, CountryCode country)
    : token_(std::move(token))
    , client_id_(std::move(client_id))
    , country_(country) {
}

bool Auth::is_valid() const {
    // Basic validation - token and client_id should not be empty
    return !token_.empty() && !client_id_.empty();
}

std::string Auth::get_auth_header() const {
    return std::format("Bearer {}", token_);
}

} // namespace thinq_proxy
