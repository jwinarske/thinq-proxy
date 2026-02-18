#pragma once

#include "error.hpp"
#include "types.hpp"
#include <string>

namespace thinq_proxy {

/**
 * @brief Authentication manager for ThinQ Connect API
 * 
 * Manages Personal Access Token (PAT) for API authentication
 */
class Auth {
public:
    /**
     * @brief Construct Auth with PAT token
     * @param pat_token Personal Access Token
     * @param country Country code
     * @param client_id Client ID
     */
    Auth(std::string pat_token, Country country, std::string client_id);
    
    /**
     * @brief Get the authorization header value
     * @return Authorization header string
     */
    std::string get_auth_header() const;
    
    /**
     * @brief Get the country code
     */
    Country get_country() const { return country_; }
    
    /**
     * @brief Get the client ID
     */
    const std::string& get_client_id() const { return client_id_; }
    
    /**
     * @brief Validate the authentication credentials
     */
    Result<bool> validate() const;

private:
    std::string pat_token_;
    Country country_;
    std::string client_id_;
};

} // namespace thinq_proxy
