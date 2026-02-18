# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial implementation of C++ ThinQ to Matter bridge
- ThinQ API client with libcurl for HTTP REST communication
- Matter bridge coordinator (placeholder for full Matter SDK integration)
- Authentication using Personal Access Token (PAT)
- Configuration management with JSON support
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

## [0.1.0] - 2024-02-18

### Initial Release
- Project structure and foundation
- Core headers and implementation files
- Build system configuration
- Documentation

[Unreleased]: https://github.com/jwinarske/thinq-proxy/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/jwinarske/thinq-proxy/releases/tag/v0.1.0
