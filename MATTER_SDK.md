# Matter SDK Integration Guide

This guide explains how to integrate the Matter SDK from [project-chip/connectedhomeip](https://github.com/project-chip/connectedhomeip) with ThinQ Proxy.

## Overview

The ThinQ Proxy uses conditional compilation to support building with or without the Matter SDK. This approach allows:
- Development and testing without Matter SDK
- Full Matter protocol support when SDK is available
- Gradual migration to complete Matter integration

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ ThinQ Proxy                                                 │
│                                                             │
│  ┌────────────────┐         ┌──────────────────┐          │
│  │  ThinQ API     │◄────────►│ Matter Bridge    │          │
│  │  Client        │         │ (Conditional)    │          │
│  └────────────────┘         └──────────────────┘          │
│                                     │                       │
│                            ┌────────┴────────┐             │
│                            │                 │             │
│                    #ifdef HAVE_MATTER_SDK    │             │
│                            │                 │             │
│                  ┌─────────▼──────┐  ┌───────▼──────┐     │
│                  │  Matter SDK    │  │  Placeholder  │     │
│                  │  Integration   │  │  (No SDK)     │     │
│                  └────────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

## Building Without Matter SDK (Default)

The default build does not require Matter SDK. This allows development of ThinQ integration:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

In this mode:
- `HAVE_MATTER_SDK` is not defined
- Matter bridge uses placeholder implementations
- All ThinQ API functionality works normally
- Device state is tracked locally but not exposed via Matter

## Building Matter SDK

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    git gcc g++ pkg-config libssl-dev libdbus-1-dev \
    libglib2.0-dev libavahi-client-dev ninja-build \
    python3-venv python3-dev python3-pip unzip \
    libgirepository1.0-dev libcairo2-dev libreadline-dev

# Install GN build tool
git clone https://gn.googlesource.com/gn
cd gn
python3 build/gen.py
ninja -C out
sudo cp out/gn /usr/local/bin/
```

### Clone and Build Matter SDK

```bash
# Clone Matter SDK
git clone --depth 1 https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Initialize submodules (this takes a while)
git submodule update --init --recursive

# Activate build environment
source scripts/activate.sh

# Build Matter SDK examples
# This builds all necessary libraries
./scripts/build/build_examples.py --target linux-x64-all-clusters build
```

This creates:
- **Include files**: Various directories under `src/`
- **Libraries**: `out/host/obj/lib/`

### Key Matter SDK Libraries

The main libraries you'll need:
- `libCHIPCoreStaticLibrary.a` - Core Matter functionality
- `libCHIPDataModel.a` - Data model and clusters
- `libCHIPAppStaticLibrary.a` - Application framework
- `libCHIPSetup.a` - Device commissioning
- `libCHIPPlatform.a` - Platform abstractions

## Building ThinQ Proxy with Matter SDK

### Method 1: Using MATTER_SDK_PATH

```bash
cd thinq-proxy
mkdir build && cd build

cmake .. \
  -DENABLE_MATTER_SDK=ON \
  -DMATTER_SDK_PATH=/path/to/connectedhomeip

cmake --build .
```

### Method 2: Install Matter SDK Systemwide

If you install Matter SDK headers and libraries to standard locations:

```bash
# Example: Copy headers and libraries (adjust paths as needed)
sudo mkdir -p /usr/local/include/matter
sudo cp -r /path/to/connectedhomeip/src/* /usr/local/include/matter/
sudo cp /path/to/connectedhomeip/out/host/obj/lib/*.a /usr/local/lib/

# Then build ThinQ Proxy
cd thinq-proxy
mkdir build && cd build
cmake .. -DENABLE_MATTER_SDK=ON
cmake --build .
```

## Implementation Status

The Matter SDK integration is currently in experimental stage:

### ✅ Completed
- CMake configuration for optional Matter SDK
- Conditional compilation structure
- Placeholder implementations
- Build system integration

### 🚧 In Progress
- Actual Matter SDK API calls
- Device cluster implementations
- Attribute handlers
- Command handlers

### 📋 Planned
- Complete thermostat cluster (Air Conditioner)
- Laundry washer cluster (Washer)
- Refrigerator cluster (Refrigerator)
- Dynamic endpoint creation
- Attribute synchronization
- Command processing

## Code Structure

### Conditional Compilation

The code uses `#ifdef HAVE_MATTER_SDK` to conditionally compile Matter SDK code:

```cpp
#ifdef HAVE_MATTER_SDK
// Actual Matter SDK implementation
#include <app/server/Server.h>
// ... Matter SDK code ...
#else
// Placeholder implementation for development
// ... No-op or logging code ...
#endif
```

### Matter Bridge Implementation

File: `src/matter/bridge.cpp`

```cpp
Expected<void> MatterBridge::initialize() {
#ifdef HAVE_MATTER_SDK
    // Initialize Matter SDK
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::Server::GetInstance().Init(...);
#else
    // Placeholder - just mark as initialized
#endif
    pimpl_->initialized = true;
    return {};
}
```

## Device Type Mappings

### Air Conditioner → Thermostat Cluster

```cpp
// ThinQ Air Conditioner maps to Matter Thermostat
Cluster: Thermostat (0x0201)

Attributes:
- LocalTemperature → ac->get_current_temperature()
- OccupiedCoolingSetpoint → ac->get_target_temperature()
- SystemMode → ac->get_mode() (Cool, Heat, Auto)
- ThermostatRunningState → ac->is_powered_on()

Commands:
- SetpointRaiseLower → ac->set_target_temperature()
```

### Washer → Laundry Washer Cluster

```cpp
// ThinQ Washer maps to Matter Laundry Washer
Cluster: LaundryWasherMode (0x0051)

Attributes:
- SupportedModes → [Normal, Delicate, Heavy]
- CurrentMode → washer->get_state()
- OperationalState → washer->get_state()

Commands:
- Start → washer->start()
- Stop → washer->stop()
```

### Refrigerator → Refrigerator Cluster

```cpp
// ThinQ Refrigerator maps to Matter Refrigerator And Temperature Controlled Cabinet Mode
Cluster: RefrigeratorAndTemperatureControlledCabinetMode (0x0052)

Attributes:
- Temperature → fridge->get_fridge_temperature()
- FreezerTemperature → fridge->get_freezer_temperature()
- DoorState → fridge->is_door_open()

Commands:
- SetTemperature → fridge->set_fridge_temperature()
```

## Troubleshooting

### "Matter SDK not found"

**Problem**: CMake can't find Matter SDK

**Solutions**:
1. Ensure `MATTER_SDK_PATH` points to the connectedhomeip root directory
2. Check that Matter SDK was built successfully
3. Verify `src/app/server/Server.h` exists in Matter SDK
4. Build without Matter SDK first: `cmake .. -DENABLE_MATTER_SDK=OFF`

### Build Errors with Matter SDK

**Problem**: Compilation errors when HAVE_MATTER_SDK is defined

**Solutions**:
1. The Matter SDK version may differ from expected
2. Adjust include paths in `src/matter/bridge.cpp`
3. Update library linking in `CMakeLists.txt`
4. Consult Matter SDK documentation for your version

### Runtime Issues

**Problem**: Bridge fails to initialize with Matter SDK

**Solutions**:
1. Ensure Matter SDK dependencies are installed
2. Check Matter SDK examples work on your system
3. Enable debug logging in Matter SDK
4. Start with Matter SDK examples before integrating

## References

- [Matter SDK Documentation](https://project-chip.github.io/connectedhomeip-doc/index.html)
- [Matter Specification](https://csa-iot.org/developer-resource/specifications-download-request/)
- [Building Matter SDK](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/BUILDING.md)
- [Matter Device Library](https://github.com/project-chip/connectedhomeip/tree/master/examples)

## Contributing

Contributions to Matter SDK integration are welcome! Please:
1. Test without Matter SDK first
2. Use conditional compilation for all Matter SDK code
3. Document any Matter SDK version-specific changes
4. Provide build instructions for your platform

## Next Steps

1. Build Matter SDK following instructions above
2. Build ThinQ Proxy with `-DENABLE_MATTER_SDK=ON`
3. Test basic Matter bridge initialization
4. Implement device-specific cluster handlers
5. Test Matter commissioning with a controller
6. Submit improvements via pull requests
