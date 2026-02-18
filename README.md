# ThinQ Connect to Matter Bridge

[![Build](https://github.com/jwinarske/thinq-proxy/actions/workflows/build.yml/badge.svg)](https://github.com/jwinarske/thinq-proxy/actions/workflows/build.yml)

A C++ bridge that exposes LG ThinQ Connect devices via Matter protocol for local smart home integration.

## Overview

This project provides a local proxy/bridge that:
- Communicates with LG ThinQ Connect API (HTTP REST)
- Exposes devices locally via Matter SDK
- Enables local smart home control without AWS MQTT

### Architecture

```
┌─────────────┐         ┌──────────────┐         ┌─────────────┐
│   ThinQ     │◄────────┤  ThinQ-Proxy │────────►│   Matter    │
│   Cloud     │  HTTPS  │   (Bridge)   │  Matter │  Controller │
│   API       │         │              │         │  (HomeKit)  │
└─────────────┘         └──────────────┘         └─────────────┘
```

## Features

- **Modern C++23** implementation
- **libcurl** for HTTP client
- **glaze** for JSON serialization (C++23 reflection)
- **Matter SDK** integration for local device exposure
- **Device Support**:
  - Air Conditioner → Matter Thermostat
  - Washer → Matter Laundry Washer
  - Refrigerator → Matter Refrigerator
  - More devices coming soon!

## Building

### Requirements

- CMake 3.25+
- GCC 13+ or Clang 16+ (C++23 support)
- libcurl development headers
- Ninja build system (recommended)

### Ubuntu/Debian

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    cmake \
    ninja-build \
    libcurl4-openssl-dev \
    pkg-config \
    build-essential \
    gcc-13 \
    g++-13

# Set GCC 13 as default
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

# Build
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-13 ..
cmake --build .
```

### macOS

```bash
# Install dependencies
brew install cmake ninja curl

# Build
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Configuration

Create a configuration file `config.txt` based on the example:

```bash
cp examples/config_example.txt config.txt
```

Edit `config.txt` with your credentials:

```
pat_token=YOUR_PAT_TOKEN_HERE
client_id=YOUR_CLIENT_ID_HERE
country=US
```

### Getting ThinQ Connect Credentials

1. Visit [LG ThinQ Connect](https://connect.lgthinq.com/)
2. Register and create a Personal Access Token (PAT)
3. Note your Client ID
4. Select your country code (US, KR, EU, UK, JP, CN)

## Usage

### Running the Bridge

```bash
./thinq-proxy [config_file]
```

If no config file is specified, it looks for `config.txt` in the current directory.

### Example Output

```
ThinQ Connect to Matter Bridge
================================

Loading configuration from: config.txt
Configuration loaded successfully
Country: US

Initializing ThinQ API client...
ThinQ API client initialized

Discovering devices...
Found 3 device(s)
  - Living Room AC (AIR_CONDITIONER)
  - Kitchen Fridge (REFRIGERATOR)
  - Laundry Washer (WASHER)

Initializing Matter bridge...
Matter bridge initialized

Adding devices to Matter bridge...
  Added: Living Room AC
  Added: Kitchen Fridge
  Added: Laundry Washer

Starting Matter bridge...
Matter bridge started successfully
Bridge is now running. Press Ctrl+C to stop.
```

## Matter Commissioning

Once the bridge is running, you can commission it with a Matter controller:

1. Open your Matter-compatible app (Apple Home, Google Home, etc.)
2. Select "Add Device" or "Add Accessory"
3. Follow the on-screen instructions to scan the QR code or enter the setup code
4. Your ThinQ devices will appear as native Matter devices

## Device Mappings

| ThinQ Device    | Matter Device Type | Supported Features                           |
|-----------------|-------------------|----------------------------------------------|
| Air Conditioner | Thermostat        | On/Off, Temperature, Mode (Cool/Heat/Dry)   |
| Washer          | Laundry Washer    | State, Remaining Time                        |
| Refrigerator    | Refrigerator      | Fridge/Freezer Temperature, Door Status     |

## Development

### Project Structure

```
thinq-proxy/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── LICENSE                  # Apache 2.0 license
├── include/                 # Public headers
│   └── thinq_proxy/
│       ├── api_client.hpp   # ThinQ API client
│       ├── auth.hpp         # Authentication
│       ├── config.hpp       # Configuration
│       ├── device.hpp       # Base device class
│       ├── error.hpp        # Error handling
│       ├── matter_bridge.hpp # Matter bridge
│       ├── types.hpp        # Core types
│       └── devices/         # Device implementations
│           ├── air_conditioner.hpp
│           ├── washer.hpp
│           └── refrigerator.hpp
├── src/                     # Implementation files
│   ├── main.cpp            # Application entry point
│   ├── thinq/              # ThinQ API implementation
│   ├── matter/             # Matter bridge implementation
│   ├── devices/            # Device implementations
│   └── config/             # Configuration loader
└── examples/               # Example code and configs
    ├── basic_bridge.cpp
    └── config_example.txt
```

### Adding a New Device

1. Create header in `include/thinq_proxy/devices/`
2. Implement in `src/devices/`
3. Add to `CMakeLists.txt`
4. Map to appropriate Matter device type in bridge

### Code Style

- Follow modern C++23 practices
- Use `std::expected` for error handling
- Prefer RAII and smart pointers
- Document public APIs with Doxygen comments

## Testing

Run the example program:

```bash
cd examples
g++-13 -std=c++23 -I../include basic_bridge.cpp -o basic_bridge
./basic_bridge
```

## CI/CD

The project uses GitHub Actions for continuous integration:
- Builds on Ubuntu latest
- Tests with GCC 13 (C++23)
- Generates artifacts

View the [build status](https://github.com/jwinarske/thinq-proxy/actions).

## Troubleshooting

### Build Errors

**"C++23 features not available"**
- Ensure you're using GCC 13+ or Clang 16+
- Verify CMake is using the correct compiler

**"curl/curl.h not found"**
- Install libcurl development headers: `sudo apt-get install libcurl4-openssl-dev`

### Runtime Errors

**"Authentication failed"**
- Verify your PAT token is correct
- Check that your client ID matches your ThinQ Connect app

**"Network error"**
- Ensure you have internet connectivity
- Check firewall settings for HTTPS (port 443)

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

Apache License 2.0 - See [LICENSE](LICENSE) file for details.

## Credits

Based on the [pythinqconnect](https://github.com/thinq-connect/pythinqconnect) project.

Key differences:
- Local Matter protocol instead of AWS MQTT
- Modern C++23 implementation
- Bridge architecture for local control

## Roadmap

- [x] Basic ThinQ API client
- [x] Matter bridge foundation
- [x] Air Conditioner support
- [x] Washer/Dryer support
- [x] Refrigerator support
- [ ] Air Purifier support
- [ ] Dishwasher support
- [ ] Enhanced status reporting
- [ ] Matter OTA updates
- [ ] Configuration UI
- [ ] Docker container
- [ ] Home Assistant integration

## Support

For issues and questions:
- Open an [issue](https://github.com/jwinarske/thinq-proxy/issues)
- Check existing [discussions](https://github.com/jwinarske/thinq-proxy/discussions)

## Disclaimer

This project is not affiliated with or endorsed by LG Electronics. Use at your own risk.
