# Build & Installation Guide

Complete instructions for building and installing the High-Performance Asynchronous Messaging Server.

## Prerequisites

### Minimum Requirements

- **C++ Compiler**: C++20 capable
  - MSVC 2019 Update 10 or later
  - GCC 10 or later
  - Clang 13 or later
- **CMake**: 3.20 or later
- **Build Tool**: Ninja, Visual Studio, or Unix Makefiles
- **Memory**: 2GB RAM minimum
- **Disk Space**: 500MB for build artifacts

### Operating System Support

- **Primary**: Windows 10/11 (WinSock2)
- **Supported**: Windows 7 SP1 (WinSock2)
- **Portable to**: Linux, macOS (requires socket layer adaptation)

---

## Windows Build

### Quick Start

```bash
# Clone repository
git clone https://github.com/MrYoniSade/HighPerformanceAsyncMessagingServer.git
cd HighPerformanceAsyncMessagingServer

# Create build directory
mkdir build
cd build

# Generate build files with Ninja (recommended)
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build .

# Run tests
ctest --output-on-failure

# Run demo
.\HighPerfServer.exe
```

### With Visual Studio IDE

```bash
# Generate solution
cmake -G "Visual Studio 17 2022" ..

# Build from command line
cmake --build . --config Release

# Or open in Visual Studio
start HighPerformanceAsyncMessagingServer.sln
```

### With Visual Studio Build Tools (CLI Only)

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release --parallel 4
```

### Troubleshooting Windows Build

**Problem**: CMake not found
```bash
# Solution: Add CMake to PATH or use full path
"C:\Program Files\CMake\bin\cmake.exe" -G "Ninja" ..
```

**Problem**: Ninja not found
```bash
# Solution 1: Install Ninja
choco install ninja

# Solution 2: Use Visual Studio generator instead
cmake -G "Visual Studio 17 2022" ..
```

**Problem**: WinSock2 errors
```bash
# Verify ws2_32.lib is in linking (automatic in CMakeLists.txt)
# If not found, add Windows SDK path to CMake
cmake -DCMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=10.0 ..
```

---

## Linux/Unix Build

### Setup

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build libgtest-dev

# Fedora
sudo dnf install -y gcc-c++ cmake ninja-build gtest-devel

# macOS
brew install cmake ninja
```

### Build (Adapter Required)

```bash
# Note: Requires adaptation of WinSock2 to BSD sockets
# For now, project is Windows-focused

mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
ctest --output-on-failure
```

### macOS Build

```bash
brew install cmake ninja
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

---

## Build Variants

### Release Build (Optimized)

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release --parallel 4
```

**Characteristics**:
- Maximum optimization (-O3)
- No debug symbols
- ~500KB executable
- Fastest execution

### Debug Build (Development)

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

**Characteristics**:
- No optimization
- Full debug symbols
- ~5MB executable
- Slow but debuggable

### RelWithDebInfo Build (Mixed)

```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build . --config RelWithDebInfo
```

**Characteristics**:
- Optimized (-O2)
- Debug symbols included
- ~2MB executable
- Good for profiling

---

## CMake Options

### Configuration Variables

```bash
# Enable/disable testing
cmake -DENABLE_TESTING=ON ..

# Set C++ standard
cmake -DCMAKE_CXX_STANDARD=20 ..

# Set optimization level
cmake -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native" ..

# Use specific compiler
cmake -DCMAKE_CXX_COMPILER=clang++-13 ..

# Set install directory
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
```

### All Configurations

```bash
# Example with multiple options
cmake -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_STANDARD=20 \
  -DCMAKE_INSTALL_PREFIX=/opt/hpams \
  ..
```

---

## Installation

### Local Installation (Optional)

```bash
# After building
cd build
cmake --install . --prefix ~/.local

# Or to system (requires sudo)
sudo cmake --install . --prefix /usr/local
```

### Verify Installation

```bash
# Check executable
./HighPerfServer --version  # (if implemented)

# Check libraries are accessible
ldconfig -p | grep hpams  # Linux
```

---

## Testing

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test

```bash
ctest -R ThreadPoolTest --output-on-failure
```

### Run with Verbose Output

```bash
ctest -V --output-on-failure
```

### Run with Timeout

```bash
ctest --timeout 10 --output-on-failure
```

### Test Coverage Report

```bash
# If built with coverage flags
cmake -DCMAKE_CXX_FLAGS="--coverage" ..
cmake --build .
ctest
gcov CMakeFiles/HighPerfServer.dir/src/*.cpp
```

---

## Benchmarking

### Run Queue Benchmark

```bash
./QueueBenchmark
```

Expected output:
```
LockFreeQueue throughput: 66.7M ops/sec
MutexQueue throughput: 33.8M ops/sec
Speedup: 1.97x
```

### Performance Profiling

```bash
# With Linux perf
perf record ./HighPerfServer
perf report

# With Windows Performance Analyzer (WPA)
# Requires Windows Performance Toolkit
wpr -start GeneralProfile
./HighPerfServer.exe
wpr -stop output.etl
```

---

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build and Test

on: [push, pull_request]

jobs:
  build:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install CMake
      uses: lukka/get-cmake@latest
    
    - name: Install Ninja
      uses: seanmiddleditch/gha-setup-ninja@master
    
    - name: Build
      run: |
        mkdir build
        cd build
        cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
        cmake --build .
    
    - name: Test
      run: |
        cd build
        ctest --output-on-failure
```

---

## Development Workflow

### Setting Up IDE

#### Visual Studio Code

```json
{
  "folders": [
    {
      "path": "."
    }
  ],
  "settings": {
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "cmake.configureOnOpen": true,
    "cmake.preferredGenerator": "Ninja",
    "cmake.buildDirectory": "${workspaceFolder}/build"
  }
}
```

#### Visual Studio

```
1. Open HighPerformanceAsyncMessagingServer.sln
2. Select Release configuration
3. Build ? Build Solution
4. Test ? Run All Tests
```

#### CLion

```
1. Open project folder
2. Select CMake build generator
3. Click Build in toolbar
4. Run tests via Test Explorer
```

---

## Troubleshooting

### Build Errors

#### "CMake: Could not find a package configuration file"

```bash
# Solution: Update CMake submodules
git submodule update --init --recursive

# Or specify GoogleTest path
cmake -DGTEST_ROOT=/path/to/gtest ..
```

#### "fatal error: gtest/gtest.h: No such file"

```bash
# Solution: Let CMake fetch GoogleTest
cmake -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=/dev/null ..
# Or install GoogleTest system-wide
```

#### Linker errors with ws2_32

```bash
# Solution: Verify Windows SDK installation
# Or explicitly link: CMakeLists.txt already includes target_link_libraries
```

### Runtime Errors

#### "Cannot bind address: Address already in use"

```bash
# Solution: Wait 60 seconds or use different port
AsyncServer server(4);
server.start("127.0.0.1", 5001);  // Different port
```

#### "Thread creation failed"

```bash
# Solution: Reduce thread pool size
ThreadPool pool(2);  // Instead of 4
```

### Performance Issues

#### Low throughput

```bash
# Check 1: Build is Release
cmake -DCMAKE_BUILD_TYPE=Release

# Check 2: Compiler optimizations enabled
cmake -DCMAKE_CXX_FLAGS="-O3 -march=native" ..

# Check 3: Profile with perf/vtune
perf record ./HighPerfServer
```

#### High CPU usage

```bash
# Reduce thread pool size
ThreadPool pool(CPU_CORES);  // Match actual cores

# Add sleep in idle loops
std::this_thread::sleep_for(std::chrono::milliseconds(1));
```

---

## Version Information

### Check Compiler Version

```bash
# MSVC
cl.exe /?

# GCC
g++ --version

# Clang
clang++ --version
```

### Check CMake Version

```bash
cmake --version
```

### Expected Output

```
cmake version 3.20.0 (or later)
MSVC: Microsoft Visual C++ 2019 (or later)
GCC: 10.0.0 (or later)
Clang: 13.0.0 (or later)
```

---

## Performance Tips for Building

### Parallel Compilation

```bash
# Use all CPU cores
cmake --build . --parallel

# Or specify number of jobs
cmake --build . -j 8
```

### Ccache (Unix-like Systems)

```bash
# Install ccache
sudo apt-get install ccache  # Ubuntu
brew install ccache          # macOS

# Use with CMake
cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
```

### Incremental Build

```bash
# Only rebuild changed files
cmake --build . --target HighPerfServer

# Faster than clean build
```

---

## Deployment

### Standalone Executable

```bash
# Copy just the executable
cp build/HighPerfServer /usr/local/bin/

# Or with installation
cmake --install . --prefix /usr/local
```

### Docker Containerization

```dockerfile
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# Install build tools
RUN choco install cmake ninja -y

# Copy source
COPY . /app
WORKDIR /app

# Build
RUN mkdir build && cd build && \
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

# Run
ENTRYPOINT ["./build/HighPerfServer.exe"]
```

---

## Support

For build issues, check:
1. CMakeLists.txt error output
2. Compiler version compatibility
3. Missing dependencies
4. System PATH configuration
5. Project GitHub issues

Visit: https://github.com/MrYoniSade/HighPerformanceAsyncMessagingServer
