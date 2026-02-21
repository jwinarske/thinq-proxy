# Building ThinQ Proxy

Quick reference for building the ThinQ Proxy project.

## Prerequisites

### Required
- **C++ Compiler**: GCC 13+, Clang 16+, or MSVC 19.35+
- **CMake**: 3.25 or later
- **libcurl**: Development headers

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libcurl4-openssl-dev
```

#### Fedora/RHEL
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    libcurl-devel
```

#### macOS
```bash
brew install cmake curl
```

## Build Steps

### 1. Clone Repository
```bash
git clone https://github.com/jwinarske/thinq-proxy.git
cd thinq-proxy
git submodule update --init --recursive
```

### 2. Create Build Directory
```bash
mkdir build
cd build
```

### 3. Configure with CMake
```bash
# Basic configuration
cmake ..

# Or with specific options
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++-13 \
    -DBUILD_EXAMPLES=ON
```

### 4. Build
```bash
# Build with all CPU cores
cmake --build . -j$(nproc)

# Or build specific target
cmake --build . --target thinq-proxy
```

### 5. Install (Optional)
```bash
sudo cmake --install .
```

## Build Targets

- `thinq-proxy` - Main application executable
- `thinq_proxy_lib` - Static library
- `basic_bridge` - Example application (if BUILD_EXAMPLES=ON)

## Building with Matter SDK

To enable full Matter protocol support, you need to build the Matter SDK first. See [MATTER_SDK.md](MATTER_SDK.md) for detailed instructions.

### Quick Start with Matter SDK

```bash
# 1. Build Matter SDK (one-time setup)
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip
git submodule update --init
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-all-clusters build

# 2. Build ThinQ Proxy with Matter SDK
cd /path/to/thinq-proxy
mkdir build && cd build
cmake .. \
    -DENABLE_MATTER_SDK=ON \
    -DMATTER_SDK_PATH=/path/to/connectedhomeip
cmake --build . -j$(nproc)
```

### Without Matter SDK (Default)

By default, the project builds without Matter SDK integration. This is useful for:
- Development and testing of ThinQ API integration
- Building on systems where Matter SDK is not available
- Faster iteration during development

```bash
cmake ..  # ENABLE_MATTER_SDK defaults to OFF
cmake --build .
```


## Build Output

After successful build:
```
build/
├── thinq-proxy              # Main executable (~236KB)
├── libthinq_proxy_lib.a     # Static library (~963KB)
└── examples/
    └── basic_bridge         # Example executable (~213KB)
```

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_EXAMPLES` | `ON` | Build example programs |
| `ENABLE_MATTER_SDK` | `OFF` | Enable Matter SDK integration |
| `MATTER_SDK_PATH` | - | Path to Matter SDK installation |
| `CMAKE_BUILD_TYPE` | `Release` | Build type (Debug/Release/RelWithDebInfo) |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory |

### Example Configurations

```bash
# Default build (no Matter SDK)
cmake ..

# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# With Matter SDK
cmake .. \
    -DENABLE_MATTER_SDK=ON \
    -DMATTER_SDK_PATH=/opt/connectedhomeip

# Custom compiler and build type
cmake .. \
    -DCMAKE_CXX_COMPILER=clang++-16 \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

## Troubleshooting

### Compiler Not Found
```bash
# Specify compiler explicitly
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-13
```

### C++23 Not Supported
Ensure you have a modern compiler:
```bash
g++ --version    # Should be 13.0 or later
clang++ --version # Should be 16.0 or later
```

### libcurl Not Found
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# Check if it's installed
pkg-config --modversion libcurl
```

### Clean Build
```bash
cd build
rm -rf *
cmake ..
cmake --build .
```

## Development Builds

For development with debug symbols:
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON  # For IDE integration

cmake --build .
```

## Cross-Compilation

Example for ARM64:
```bash
cmake .. \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
```

## Testing the Build

```bash
# Check executable
./thinq-proxy --help

# Run with test config
cp ../examples/config_example.json config.json
# Edit config.json with your credentials
./thinq-proxy config.json
```

## Next Steps

After successful build:
1. Copy `examples/config_example.json` to `config.json`
2. Edit with your ThinQ credentials
3. Run `./thinq-proxy config.json`
4. Commission with your Matter controller

See [README.md](README.md) for detailed usage instructions.
