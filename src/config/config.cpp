// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include "thinq_proxy/config.hpp"
#include <fstream>
#include <format>

namespace thinq_proxy {

Expected<Config> Config::load(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return make_error<Config>(ErrorCode::ConfigError,
                                  std::format("Config file not found: {}", path.string()));
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return make_error<Config>(ErrorCode::ConfigError,
                                  std::format("Failed to open config file: {}", path.string()));
    }
    
    // TODO: Parse JSON using glaze
    // For now, return a default config
    Config config;
    
    // Placeholder - in real implementation, parse JSON
    // Expected format:
    // {
    //   "thinq": {
    //     "pat_token": "...",
    //     "client_id": "...",
    //     "country": "US",
    //     "poll_interval_sec": 5
    //   },
    //   "matter": {
    //     "bridge_name": "ThinQ Bridge",
    //     "vendor_id": 65521,
    //     "product_id": 32769,
    //     "discriminator": 3840,
    //     "setup_passcode": 20202021,
    //     "port": 5540
    //   }
    // }
    
    return make_success(std::move(config));
}

Expected<void> Config::save(const std::filesystem::path& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return make_error<void>(ErrorCode::ConfigError,
                                std::format("Failed to create config file: {}", path.string()));
    }
    
    // TODO: Serialize to JSON using glaze
    // For now, write a basic JSON structure
    file << R"({
  "thinq": {
    "pat_token": "",
    "client_id": "",
    "country": "US",
    "poll_interval_sec": 5
  },
  "matter": {
    "bridge_name": "ThinQ Bridge",
    "vendor_id": 65521,
    "product_id": 32769,
    "discriminator": 3840,
    "setup_passcode": 20202021,
    "port": 5540
  }
})";
    
    return {};
}

bool Config::is_valid() const {
    return !thinq_config.pat_token.empty() && 
           !thinq_config.client_id.empty() &&
           thinq_config.poll_interval_sec > 0;
}

} // namespace thinq_proxy
