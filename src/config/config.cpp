// Copyright 2024 ThinQ Proxy Contributors
// Licensed under the Apache License, Version 2.0

#include <cstdint>

#include <spdlog/spdlog.h>

#include "glaze/glaze.hpp"

#include "thinq_proxy/config.hpp"

namespace thinq_proxy {

struct ThinQConfigFile {
  std::string pat_token;
  std::string client_id;
  std::string country{"US"};
  int poll_interval_sec{5};
};

struct MatterConfigFile {
  std::string bridge_name{"ThinQ Bridge"};
  uint16_t vendor_id{0xFFF1};
  uint16_t product_id{0x8001};
  uint16_t discriminator{3840};
  uint32_t setup_passcode{20202021};
  uint16_t port{5540};
};

struct ConfigFile {
  ThinQConfigFile thinq;
  MatterConfigFile matter;
};

namespace {

Expected<CountryCode> parse_country_code(std::string_view country) {
  if (country == "US") return make_success(CountryCode::US);
  if (country == "KR") return make_success(CountryCode::KR);
  if (country == "EU") return make_success(CountryCode::EU);
  if (country == "GB") return make_success(CountryCode::GB);
  if (country == "CA") return make_success(CountryCode::CA);
  if (country == "AU") return make_success(CountryCode::AU);
  if (country == "JP") return make_success(CountryCode::JP);

  return make_error<CountryCode>(
    ErrorCode::ConfigError,
    std::format("Invalid country code '{}'. Supported: US, KR, EU, GB, CA, AU, JP", country)
  );
}

bool is_valid_setup_passcode(uint32_t setup_passcode) {
  return setup_passcode >= 10000000 && setup_passcode <= 99999999;
}

void print_config_file(const ConfigFile& config_file) {
  auto printable = config_file;
  if (!printable.thinq.pat_token.empty()) {
    printable.thinq.pat_token = "***REDACTED***";
  }

  const auto json_result = glz::write_json(printable);
  if (!json_result) {
    spdlog::error("ConfigFile serialization failed: {}", glz::format_error(json_result));
    return;
  }

  std::string pretty_json;
  glz::prettify_json(json_result.value(), pretty_json);
  spdlog::info("ConfigFile:\n{}", pretty_json);
}

} // namespace

} // namespace thinq_proxy

namespace glz {

template <>
struct meta<thinq_proxy::ThinQConfigFile> {
  using T = thinq_proxy::ThinQConfigFile;
  static constexpr auto value = object(
    "pat_token", &T::pat_token,
    "client_id", &T::client_id,
    "country", &T::country,
    "poll_interval_sec", &T::poll_interval_sec
  );
};

template <>
struct meta<thinq_proxy::MatterConfigFile> {
  using T = thinq_proxy::MatterConfigFile;
  static constexpr auto value = object(
    "bridge_name", &T::bridge_name,
    "vendor_id", &T::vendor_id,
    "product_id", &T::product_id,
    "discriminator", &T::discriminator,
    "setup_passcode", &T::setup_passcode,
    "port", &T::port
  );
};

template <>
struct meta<thinq_proxy::ConfigFile> {
  using T = thinq_proxy::ConfigFile;
  static constexpr auto value = object(
    "thinq", &T::thinq,
    "matter", &T::matter
  );
};

} // namespace glz

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

    std::string json_data{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    ConfigFile config_file;
    if (const auto ec = glz::read_json(config_file, json_data); ec) {
      return make_error<Config>(
        ErrorCode::ConfigError,
        std::format("Failed to parse config JSON: {}", glz::format_error(ec, json_data))
      );
    }

    print_config_file(config_file);

    auto country_result = parse_country_code(config_file.thinq.country);
    if (!country_result) {
      return make_error<Config>(ErrorCode::ConfigError, country_result.error().message);
    }

    Config config;
    config.thinq_config.pat_token = std::move(config_file.thinq.pat_token);
    config.thinq_config.client_id = std::move(config_file.thinq.client_id);
    config.thinq_config.country = country_result.value();
    config.thinq_config.poll_interval_sec = config_file.thinq.poll_interval_sec;

    config.matter_config.bridge_name = std::move(config_file.matter.bridge_name);
    config.matter_config.vendor_id = config_file.matter.vendor_id;
    config.matter_config.product_id = config_file.matter.product_id;
    config.matter_config.discriminator = config_file.matter.discriminator;
    config.matter_config.setup_passcode = config_file.matter.setup_passcode;
    config.matter_config.port = config_file.matter.port;

    if (!config.is_valid()) {
      return make_error<Config>(
        ErrorCode::ConfigError,
        "Invalid configuration values. Check required fields and ranges."
      );
    }

    return make_success(std::move(config));
}

Expected<void> Config::save(const std::filesystem::path& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        return make_error<void>(ErrorCode::ConfigError,
                                std::format("Failed to create config file: {}", path.string()));
    }

    ConfigFile config_file;
    config_file.thinq.pat_token = thinq_config.pat_token;
    config_file.thinq.client_id = thinq_config.client_id;
    config_file.thinq.country = std::string(to_string(thinq_config.country));
    config_file.thinq.poll_interval_sec = thinq_config.poll_interval_sec;

    config_file.matter.bridge_name = matter_config.bridge_name;
    config_file.matter.vendor_id = matter_config.vendor_id;
    config_file.matter.product_id = matter_config.product_id;
    config_file.matter.discriminator = matter_config.discriminator;
    config_file.matter.setup_passcode = matter_config.setup_passcode;
    config_file.matter.port = matter_config.port;

    std::string json_data;
    if (const auto ec = glz::write_json(config_file, json_data); ec) {
        return make_error<void>(
            ErrorCode::ConfigError,
            std::format("Failed to serialize config JSON: {}", glz::format_error(ec, json_data))
        );
    }

    file << json_data << '\n';
    
    return {};
}

bool Config::is_valid() const {
    return !thinq_config.pat_token.empty() && 
           !thinq_config.client_id.empty() &&
           thinq_config.poll_interval_sec > 0 &&
           !matter_config.bridge_name.empty() &&
           matter_config.discriminator <= 4095 &&
           matter_config.port > 0 &&
           is_valid_setup_passcode(matter_config.setup_passcode);
}

} // namespace thinq_proxy
