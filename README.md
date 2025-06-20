# yaml2json

A YAML to JSON converter implemented using the RapidYAML library, designed for high-throughput data processing applications.

## Performance Analysis

Comparative performance measurements were conducted using [hyperfine](https://github.com/sharkdp/hyperfine) on Apple Silicon (macOS 14.5.0) against established tools yq and lq:

| File Size | yaml2json | yq | lq | Relative Performance |
|-----------|-----------|----|----|---------------------|
| 117KB | 2.1ms ± 0.3ms | 22.9ms ± 0.9ms | 20.0ms ± 0.2ms | 10.7x / 9.4x improvement |
| 0.99MB | 12.5ms ± 0.6ms | 169.4ms ± 5.1ms | 145.5ms ± 0.9ms | 13.5x / 11.6x improvement |
| 6.53MB | 68.8ms ± 1.2ms | 1.012s ± 0.042s | 911.8ms ± 48.5ms | 14.7x / 13.3x improvement |
| 13.23MB | 139.8ms ± 1.1ms | 2.064s ± 0.053s | 1.857s ± 0.042s | 14.8x / 13.3x improvement |

### Implementation Characteristics

- Zero-copy parsing using memory-mapped file input
- Compiler optimizations: `-Ofast -flto=thin -march=native`
- Direct JSON serialization without intermediate data structures
- Linear time complexity scaling with input size
- Deterministic parsing behavior across test cases
- Consistent performance scaling (10-15x improvement ratio maintained across file sizes)

### Benchmark Methodology

Performance measurements can be reproduced using the provided test suite:

```bash
# Repository setup
git clone https://github.com/SimpleSoftwareOrg/yaml2json.git
cd yaml2json

# Build configuration
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Benchmark execution
cd ../benchmarks
./generate_compatible_yaml.sh  # Generate standardized test files
./benchmark.sh                 # Execute comparative measurements
```

**Dependencies**: hyperfine, yq, and lq. Dependency validation is performed automatically.

**Test Protocol**: Each measurement consists of 3 warmup iterations followed by 10 timed executions using hyperfine. Statistical analysis includes mean execution time, standard deviation, and confidence intervals.

## Installation

### Binary Distributions

Pre-compiled binaries are available for multiple platforms:

| Platform | Architecture | Download |
|----------|-------------|----------|
| macOS | ARM64 (Apple Silicon) | [yaml2json-macos-arm64](https://github.com/SimpleSoftwareOrg/yaml2json/releases/latest/download/yaml2json-macos-arm64) |
| macOS | x86_64 (Intel) | [yaml2json-macos-x86_64](https://github.com/SimpleSoftwareOrg/yaml2json/releases/latest/download/yaml2json-macos-x86_64) |
| Linux | x86_64 | [yaml2json-linux-x86_64](https://github.com/SimpleSoftwareOrg/yaml2json/releases/latest/download/yaml2json-linux-x86_64) |
| Linux | ARM64 | [yaml2json-linux-arm64](https://github.com/SimpleSoftwareOrg/yaml2json/releases/latest/download/yaml2json-linux-arm64) |
| Windows | x86_64 | [yaml2json-windows-x86_64.exe](https://github.com/SimpleSoftwareOrg/yaml2json/releases/latest/download/yaml2json-windows-x86_64.exe) |

### Installation Script

```bash
# System-wide installation (/usr/local/bin)
curl -sSL https://raw.githubusercontent.com/SimpleSoftwareOrg/yaml2json/main/install.sh | sudo bash

# User installation (~/bin)
curl -sSL https://raw.githubusercontent.com/SimpleSoftwareOrg/yaml2json/main/install.sh | bash -s -- --user
```



## Usage

```bash
# File conversion
yaml2json input.yaml > output.json

# Pipeline processing
cat input.yaml | yaml2json > output.json

# Batch processing
yaml2json input.yaml > output.json
```

## Technical Specifications

- Zero-copy parsing implementation using memory-mapped file I/O
- Link-time optimization (LTO) with platform-specific instruction tuning
- Direct JSON serialization to stdout
- Support for files up to available system memory
- Cross-platform compatibility (macOS, Linux, Windows)

## Building from Source

### Build Requirements

- CMake 3.15 or later
- C++17-compliant compiler (GCC 8+, Clang 8+, MSVC 2019+)

### Compilation

```bash
git clone https://github.com/SimpleSoftwareOrg/yaml2json.git
cd yaml2json
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Build Configuration

- `CMAKE_BUILD_TYPE=Release` - Production build with optimizations
- `CMAKE_BUILD_TYPE=Debug` - Development build with debug information
- `YAML2JSON_STATIC` - Static linking configuration

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for complete terms.

## Contributing

Contributions are accepted following the guidelines in [CONTRIBUTING.md](CONTRIBUTING.md). 