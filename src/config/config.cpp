#include "thinq_proxy/config.hpp"
#include <fstream>
#include <sstream>

namespace thinq_proxy {

Result<Config> Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return std::unexpected(make_error_code(ErrorCode::InvalidConfiguration));
    }
    
    Config config;
    
    // Simple parsing - in real implementation would use glaze for JSON
    // For now, this is a placeholder that expects specific format
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        // Simple key=value parsing
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (key == "pat_token") config.pat_token = value;
            else if (key == "client_id") config.client_id = value;
            else if (key == "country") {
                if (value == "US") config.country = Country::US;
                else if (value == "KR") config.country = Country::KR;
                else if (value == "EU") config.country = Country::EU;
                else if (value == "UK") config.country = Country::UK;
                else if (value == "JP") config.country = Country::JP;
                else if (value == "CN") config.country = Country::CN;
            }
        }
    }
    
    auto valid = config.validate();
    if (!valid) {
        return std::unexpected(valid.error());
    }
    
    return config;
}

Result<void> Config::save(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return std::unexpected(make_error_code(ErrorCode::InvalidConfiguration));
    }
    
    file << "# ThinQ Proxy Configuration\n";
    file << "pat_token=" << pat_token << "\n";
    file << "client_id=" << client_id << "\n";
    file << "country=" << country_to_string(country) << "\n";
    
    return {};
}

Result<void> Config::validate() const {
    if (pat_token.empty()) {
        return std::unexpected(make_error_code(ErrorCode::InvalidConfiguration));
    }
    if (client_id.empty()) {
        return std::unexpected(make_error_code(ErrorCode::InvalidConfiguration));
    }
    return {};
}

} // namespace thinq_proxy
