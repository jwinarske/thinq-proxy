# Contributing to ThinQ Proxy

Thank you for your interest in contributing to ThinQ Proxy! This document provides guidelines and information for contributors.

## Code of Conduct

This project follows a standard Code of Conduct. Be respectful and professional in all interactions.

## How to Contribute

### Reporting Issues

- Use GitHub Issues to report bugs or request features
- Include detailed information:
  - Operating system and version
  - Compiler version
  - Steps to reproduce
  - Expected vs actual behavior
  - Relevant logs or error messages

### Submitting Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes following the coding style
4. Test your changes
5. Commit with clear messages (`git commit -m 'Add amazing feature'`)
6. Push to your branch (`git push origin feature/amazing-feature`)
7. Open a Pull Request

## Development Setup

### Prerequisites

- C++23 compatible compiler (GCC 13+, Clang 16+)
- CMake 3.25+
- libcurl development headers
- Git

### Building

```bash
git clone https://github.com/jwinarske/thinq-proxy.git
cd thinq-proxy
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## Coding Standards

### C++ Style

- Use modern C++23 features where appropriate
- Follow existing code style
- Use meaningful variable and function names
- Keep functions focused and small
- Prefer `const` and `constexpr` where possible

### Code Structure

```cpp
// Copyright header
// Licensed under Apache 2.0

#pragma once  // For headers

#include <system_headers>
#include "project_headers.hpp"

namespace thinq_proxy {

// Class/function definitions
// Use Doxygen-style comments for public APIs

} // namespace thinq_proxy
```

### Error Handling

- Use `std::expected<T, Error>` for operations that can fail
- Avoid exceptions in core library code
- Provide meaningful error messages

### Naming Conventions

- Classes: `PascalCase` (e.g., `ApiClient`)
- Functions/methods: `snake_case` (e.g., `get_devices()`)
- Member variables: `snake_case_` with trailing underscore (e.g., `token_`)
- Constants: `UPPER_SNAKE_CASE` or `kPascalCase`
- Enums: `PascalCase` for type, `PascalCase` for values

### Comments

- Use `///` for Doxygen documentation
- Use `//` for inline comments
- Document public APIs
- Explain "why" not "what" in complex code

Example:
```cpp
/**
 * @brief Get list of all devices
 * @return List of device information or error
 */
Expected<std::vector<DeviceInfo>> get_devices();
```

## Testing

Currently, the project focuses on implementation. When adding tests:

- Use a consistent testing framework
- Test both success and failure cases
- Mock external dependencies (ThinQ API, Matter SDK)
- Aim for good code coverage

## Adding New Device Types

1. Create header in `include/thinq_proxy/your_device.hpp`
2. Inherit from `Device` base class
3. Implement device-specific methods
4. Create implementation in `src/devices/your_device.cpp`
5. Add to `CMakeLists.txt`
6. Update documentation

Example:
```cpp
class AirPurifier : public Device {
public:
    using Device::Device;
    
    // Device-specific methods
    Expected<void> set_fan_speed(int speed);
    int get_air_quality() const;
    
protected:
    void parse_status(const std::string& json_data) override;
    
private:
    int fan_speed_{1};
    int air_quality_{0};
};
```

## Matter Device Mapping

When adding device support, document the Matter device type mapping:

| ThinQ Device | Matter Device Type | Status |
|--------------|-------------------|--------|
| Your Device  | Matter Type       | 🚧     |

## Documentation

- Update README.md for user-facing changes
- Update this CONTRIBUTING.md for development changes
- Add inline documentation for new APIs
- Include code examples where helpful

## Commit Messages

Use conventional commits format:

```
feat: Add air purifier support
fix: Handle network timeout properly
docs: Update build instructions
refactor: Simplify device factory
test: Add API client tests
chore: Update dependencies
```

## Review Process

- All changes require review before merging
- Address review comments promptly
- Keep PRs focused and reasonably sized
- Update PR based on feedback

## License

By contributing, you agree that your contributions will be licensed under the Apache License 2.0.

## Questions?

- Open a GitHub Discussion for general questions
- Open an Issue for specific problems
- Contact maintainers for other inquiries

Thank you for contributing to ThinQ Proxy!
