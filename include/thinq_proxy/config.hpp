// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#pragma once

#include "thinq_proxy/types.hpp"
#include "thinq_proxy/error.hpp"
#include "thinq_proxy/matter_bridge.hpp"
#include <string>
#include <filesystem>

namespace thinq_proxy {

/**
 * @brief Configuration management for ThinQ Proxy
 * 
 * Handles loading and saving configuration from JSON files.
 */
class Config {
public:
    /// ThinQ API configuration
    struct ThinQConfig {
        std::string pat_token;
        std::string client_id;
        CountryCode country{CountryCode::US};
        int poll_interval_sec{5};
    };
    
    /// Matter bridge configuration
    MatterBridge::Config matter_config;
    
    /// ThinQ configuration
    ThinQConfig thinq_config;
    
    /**
     * @brief Load configuration from JSON file
     * @param path Path to configuration file
     * @return Config object or error
     */
    static Expected<Config> load(const std::filesystem::path& path);
    
    /**
     * @brief Save configuration to JSON file
     * @param path Path to save configuration
     * @return Success or error
     */
    Expected<void> save(const std::filesystem::path& path) const;
    
    /**
     * @brief Validate configuration
     * @return true if configuration is valid
     */
    bool is_valid() const;
};

} // namespace thinq_proxy
