# ThinQ to Matter Bridge

A C++23 bridge that exposes LG ThinQ Connect devices via the Matter protocol for local smart home integration.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

## Overview

This project provides a local proxy/bridge that connects LG ThinQ Connect smart home devices to the Matter ecosystem. Unlike cloud-based solutions, this bridge enables direct local control of your ThinQ devices through Matter controllers (Apple Home, Google Home, Amazon Alexa, etc.).

### Key Features

- 🔌 **Local Control**: Direct Matter protocol exposure without cloud dependencies
- 🚀 **Modern C++23**: Leverages latest C++ features for safety and performance
- 🏠 **Multiple Devices**: Support for Air Conditioners, Washers, Refrigerators, and more
- 🔄 **Real-time Sync**: Bidirectional state synchronization between ThinQ and Matter
- 🛡️ **Type Safety**: Uses `std::expected` for error handling without exceptions
- 📦 **Easy Setup**: Simple JSON configuration

## Architecture

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│  Matter         │         │  ThinQ Bridge    │         │  LG ThinQ       │
│  Controllers    │◄───────►│  (This Project)  │◄───────►│  Cloud API      │
│  (Local)        │  Matter │                  │  HTTPS  │                 │
└─────────────────┘         └──────────────────┘         └─────────────────┘
                                     │
                                     │ Controls
                                     ▼
                            ┌──────────────────┐
                            │  ThinQ Devices   │
                            │  (AC, Washer,    │
                            │   Fridge, etc.)  │
                            └──────────────────┘
```

**Components:**
1. **ThinQ API Client**: Communicates with LG ThinQ Connect API via HTTP/REST
2. **Matter Bridge**: Exposes devices locally using Matter SDK
3. **Device Implementations**: Device-specific logic for state mapping and control

## Supported Devices

| ThinQ Device      | Matter Device Type     | Status    |
|-------------------|------------------------|-----------|
| Air Conditioner   | Thermostat             | ✅ Implemented |
| Washer            | Laundry Washer         | ✅ Implemented |
| Refrigerator      | Refrigerator           | ✅ Implemented |
| Dryer             | Laundry Dryer          | 🚧 Planned |
| Air Purifier      | Air Purifier           | 🚧 Planned |
| Dishwasher        | Dishwasher             | 🚧 Planned |

## Requirements

### Build Dependencies

- **C++ Compiler**: GCC 13+, Clang 16+, or MSVC 19.35+ with C++23 support
- **CMake**: 3.25 or later
- **libcurl**: For HTTP requests
- **Matter SDK** (optional): For Matter protocol support

### Runtime Requirements

- **ThinQ Developer Account**: [Register here](https://smartsolution.developer.lge.com/en/apiManage/thinq_connect)
- **Personal Access Token (PAT)**: Generate from ThinQ Developer Portal
- **Client ID**: From ThinQ Developer Portal

## Matter SDK Integration

The ThinQ Proxy can be built with or without the Matter SDK:

### Option 1: Build Without Matter SDK (Default)

By default, the project builds without Matter SDK integration. This allows you to:
- Develop and test ThinQ API integration
- Test device discovery and control
- Prepare for Matter integration

The Matter bridge will use placeholder implementations that log device state locally without actual Matter protocol support.

```bash
cmake ..
cmake --build .
```

### Option 2: Build With Matter SDK

To enable full Matter protocol support, you need to build and install the Matter SDK from [project-chip/connectedhomeip](https://github.com/project-chip/connectedhomeip) first.

#### Building Matter SDK

```bash
# Clone the Matter SDK repository
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Initialize submodules
git submodule update --init

# Activate environment
source scripts/activate.sh

# Build for Linux (example for x86_64)
# This creates libraries in out/host
./scripts/build/build_examples.py --target linux-x64-all-clusters build

# The built libraries will be in:
# - Include files: src/ (various subdirectories)
# - Libraries: out/host/obj/lib/
```

#### Building ThinQ Proxy with Matter SDK

Once the Matter SDK is built, configure ThinQ Proxy to use it:

```bash
cd thinq-proxy
mkdir build && cd build

# Option A: Set MATTER_SDK_PATH to your Matter SDK build
cmake .. \
  -DENABLE_MATTER_SDK=ON \
  -DMATTER_SDK_PATH=/path/to/connectedhomeip

# Option B: Install Matter SDK to standard location
# (If Matter SDK is installed to /usr/local or /usr)
cmake .. -DENABLE_MATTER_SDK=ON

cmake --build .
```

**Note:** Matter SDK integration is experimental. The actual implementation depends on your Matter SDK version and may require adjustments to the source code.

## Building

### Quick Start

```bash
# Clone the repository
git clone https://github.com/jwinarske/thinq-proxy.git
cd thinq-proxy

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . -j$(nproc)

# Install (optional)
sudo cmake --install .
```

### CMake Options

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON \
  -DENABLE_MATTER_SDK=OFF  # Set to ON to enable Matter SDK (requires pre-built SDK)
```

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_EXAMPLES` | `ON` | Build example programs |
| `ENABLE_MATTER_SDK` | `OFF` | Enable Matter SDK integration |
| `MATTER_SDK_PATH` | - | Path to Matter SDK installation |
| `CMAKE_BUILD_TYPE` | `Release` | Build type (Debug/Release/RelWithDebInfo) |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory |

## Configuration

### 1. Create Configuration File

Copy the example configuration:

```bash
cp examples/config_example.json config.json
```

### 2. Edit Configuration

```json
{
  "thinq": {
    "pat_token": "YOUR_PERSONAL_ACCESS_TOKEN",
    "client_id": "YOUR_CLIENT_ID",
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
}
```

**Configuration Fields:**

- `pat_token`: Personal Access Token from LG ThinQ Developer Portal
- `client_id`: Client ID from LG ThinQ Developer Portal
- `country`: Country code (US, KR, EU, GB, CA, AU, JP)
- `poll_interval_sec`: How often to poll ThinQ API for updates (seconds)
- `bridge_name`: Name shown in Matter controllers
- `setup_passcode`: Matter commissioning passcode (8 digits)
- `discriminator`: Matter discriminator (0-4095)
- `port`: Matter server port

### 3. Getting ThinQ Credentials

1. Visit [LG ThinQ Connect Developer Portal](https://connect.lgthinq.com/)
2. Sign up for a developer account
3. Create a new application
4. Generate a Personal Access Token (PAT)
5. Note your Client ID

## Running

### Start the Bridge

```bash
./thinq-proxy [config.json]
```

The bridge will:
1. Connect to ThinQ API
2. Discover your devices
3. Initialize Matter bridge
4. Start advertising for commissioning

### Commissioning with Matter Controllers

#### Apple Home (iOS/macOS)

1. Open Home app
2. Tap "+" → "Add Accessory"
3. Scan QR code or enter setup code: `20202021`
4. Select "Add Anyway" when prompted about uncertified accessory

#### Google Home

1. Open Google Home app
2. Tap "+" → "Set up device" → "Works with Google"
3. Search for "Matter"
4. Enter setup code: `20202021`

#### Amazon Alexa

1. Open Alexa app
2. Tap "Devices" → "+" → "Add Device"
3. Select "Matter"
4. Follow on-screen instructions with setup code: `20202021`

## Usage Examples

### Basic Bridge Setup

```cpp
#include "thinq_proxy/auth.hpp"
#include "thinq_proxy/api_client.hpp"
#include "thinq_proxy/matter_bridge.hpp"

using namespace thinq_proxy;

int main() {
    // Create authentication
    auto auth = std::make_shared<Auth>(
        "YOUR_PAT_TOKEN",
        "YOUR_CLIENT_ID",
        CountryCode::US
    );
    
    // Create API client
    auto api_client = std::make_shared<ApiClient>(auth);
    
    // Discover devices
    auto devices = api_client->get_devices();
    
    // Create and start bridge
    MatterBridge bridge;
    bridge.initialize();
    
    for (auto& device_info : devices.value()) {
        auto device = create_device(device_info, api_client);
        bridge.add_device(device);
    }
    
    bridge.start();
    
    // Main loop
    while (true) {
        bridge.sync_state();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
```

See `examples/` directory for more examples.

## Development

### Project Structure

```
thinq-proxy/
├── include/thinq_proxy/     # Public headers
│   ├── types.hpp            # Core types and enums
│   ├── error.hpp            # Error handling (std::expected)
│   ├── auth.hpp             # Authentication
│   ├── api_client.hpp       # ThinQ API client
│   ├── device.hpp           # Base device class
│   ├── matter_bridge.hpp    # Matter bridge
│   ├── config.hpp           # Configuration
│   ├── air_conditioner.hpp  # Device implementations
│   ├── washer.hpp
│   └── refrigerator.hpp
├── src/                     # Implementation files
│   ├── thinq/              # ThinQ API implementation
│   ├── matter/             # Matter bridge implementation
│   ├── devices/            # Device-specific implementations
│   ├── config/             # Configuration management
│   └── main.cpp            # Main application
├── examples/               # Example programs
└── CMakeLists.txt         # Build configuration
```

### C++23 Features Used

- **`std::expected`**: Type-safe error handling
- **`std::format`**: String formatting
- **Concepts**: Type constraints
- **Ranges**: Collection processing
- **Designated Initializers**: Clear struct initialization

### Adding New Device Types

1. Create header in `include/thinq_proxy/your_device.hpp`
2. Inherit from `Device` base class
3. Implement device-specific methods
4. Add Matter device type mapping
5. Register in device factory

## Troubleshooting

### Build Issues

**Problem**: `C++23 features not available`
```bash
# Ensure you have a modern compiler
g++ --version  # Should be 13+
clang++ --version  # Should be 16+
```

**Problem**: `libcurl not found`
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# macOS
brew install curl

# Fedora/RHEL
sudo dnf install libcurl-devel
```

### Runtime Issues

**Problem**: `Authentication failed`
- Verify your PAT token is correct and not expired
- Check client ID matches your application
- Ensure country code is correct

**Problem**: `No devices found`
- Verify devices are registered in LG ThinQ app
- Check devices are online in ThinQ app
- Ensure PAT token has proper permissions

**Problem**: `Matter commissioning fails`
- Check firewall allows UDP port 5540
- Verify setup passcode is correct
- Try restarting the bridge

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

Copyright 2024 ThinQ Proxy Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

## Acknowledgments

- Based on [pythinqconnect](https://github.com/thinq-connect/pythinqconnect) by LG Electronics
- Uses [glaze](https://github.com/stephenberry/glaze) for JSON serialization
- Inspired by the Matter protocol and smart home integration standards

## Related Projects

- [pythinqconnect](https://github.com/thinq-connect/pythinqconnect) - Python implementation
- [Matter SDK](https://github.com/project-chip/connectedhomeip) - Matter protocol implementation
- [LG ThinQ Developer Portal](https://connect.lgthinq.com/) - API documentation

## Support

- **Issues**: [GitHub Issues](https://github.com/jwinarske/thinq-proxy/issues)
- **Discussions**: [GitHub Discussions](https://github.com/jwinarske/thinq-proxy/discussions)
- **API Docs**: [LG ThinQ Connect API](https://connect.lgthinq.com/en/develop/api/introduction)
