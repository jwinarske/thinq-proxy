// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "types.hpp"
#include "error.hpp"
#include <string>
#include <chrono>

namespace thinq_proxy {

/**
 * @brief Manages Personal Access Token (PAT) authentication for ThinQ API
 * 
 * The Auth class handles storing and providing authentication credentials
 * for API requests. Currently supports PAT-based authentication.
 */
class Auth {
public:
    /**
     * @brief Construct Auth with PAT token
     * @param token Personal Access Token from LG ThinQ Developer Portal
     * @param client_id Client ID from LG ThinQ Developer Portal
     * @param country Country code for API endpoint selection
     */
    Auth(std::string token, std::string client_id, CountryCode country);

    /// Get the PAT token
    const std::string& token() const { return token_; }
    
    /// Get the client ID
    const std::string& client_id() const { return client_id_; }
    
    /// Get the country code
    CountryCode country() const { return country_; }
    
    /// Check if token is valid (basic validation)
    bool is_valid() const;
    
    /// Get authorization header value
    std::string get_auth_header() const;

private:
    std::string token_;
    std::string client_id_;
    CountryCode country_;
};

} // namespace thinq_proxy
