# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Optional Matter SDK integration from project-chip/connectedhomeip
- CMake option `ENABLE_MATTER_SDK` to enable/disable Matter SDK
- Conditional compilation support for Matter SDK features
- `MATTER_SDK_PATH` CMake variable for custom SDK locations
- Comprehensive Matter SDK integration documentation (MATTER_SDK.md)
- Matter SDK build instructions in README and BUILD.md
- Placeholder implementations that work without Matter SDK

### Changed
- Matter bridge now supports conditional compilation
- Updated CMakeLists.txt to find and link Matter SDK optionally
- Refactored Matter bridge implementation for SDK integration
- Enhanced build configuration output to show Matter SDK status

### Technical Details
- Uses `#ifdef HAVE_MATTER_SDK` for conditional compilation
- Searches for Matter SDK in standard locations and custom paths
- Maintains backward compatibility (builds without Matter SDK by default)
- Documented device cluster mappings (Thermostat, Laundry Washer, Refrigerator)

## [0.1.0] - Previous Release

### Added
- Initial implementation of C++ ThinQ to Matter bridge
- ThinQ API client with libcurl for HTTP REST communication
- Matter bridge coordinator (placeholder for full Matter SDK integration)
- Device implementations:
  - Air Conditioner → Matter Thermostat
  - Washer → Matter Laundry Washer
  - Refrigerator → Matter Refrigerator
- Main application with device discovery and sync loop
- CMake build system with C++23 support
- Comprehensive README documentation
- Example code and configuration
- CONTRIBUTING guidelines

### Technical Details
- Uses modern C++23 features:
  - `std::expected` for error handling
  - `std::format` for string formatting
  - Designated initializers
  - `[[maybe_unused]]` attribute
- Header-only design for error handling and types
- PIMPL pattern for API implementation details
- Template-based error handling helpers

[Unreleased]: https://github.com/jwinarske/thinq-proxy/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/jwinarske/thinq-proxy/releases/tag/v0.1.0
