# yaml2json

A YAML to JSON converter implemented using the RapidYAML library, designed for high-throughput data processing applications.

## Performance Analysis

Comparative performance measurements were conducted using [hyperfine](https://github.com/sharkdp/hyperfine) on Apple Silicon (macOS 14.5.0) against established tools yq and lq:

| File Size | yaml2json | yq | lq | Relative Performance |
|-----------|-----------|----|----|---------------------|
| 117KB | 3.1ms ± 0.7ms | 22.8ms ± 0.5ms | 20.4ms ± 0.4ms | 7.3x / 6.6x improvement |
| 0.99MB | 12.2ms ± 1.0ms | 168.9ms ± 2.9ms | 150.0ms ± 20.8ms | 13.8x / 12.3x improvement |
| 6.53MB | 68.4ms ± 0.4ms | 992.0ms ± 45.4ms | 898.4ms ± 25.1ms | 14.5x / 13.1x improvement |
| 13.23MB | 140.4ms ± 2.7ms | 2.078s ± 0.063s | 1.856s ± 0.048s | 14.8x / 13.2x improvement |

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
./benchmark.sh                 # Execute comparative measurements (auto-generates test files)
```

**Dependencies**: hyperfine and yq (lq is downloaded automatically). Dependency validation is performed automatically.

**Test Protocol**: Each measurement consists of 3 warmup iterations followed by 20 timed executions using hyperfine. Statistical analysis includes mean execution time, standard deviation, and confidence intervals.

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
# Basic file conversion
yaml2json --input input.yaml --output output.json

# Short form options
yaml2json -i input.yaml -o output.json

# Pretty-printed JSON output
yaml2json --input input.yaml --output output.json --pretty

# Show help
yaml2json --help

# Show version (build date)
yaml2json --version
```

### Command-Line Options

| Option | Short | Description | Required |
|--------|-------|-------------|----------|
| `--input` | `-i` | Input YAML file path | Yes |
| `--output` | `-o` | Output JSON file path | Yes |
| `--pretty` | `-p` | Pretty-print JSON with indentation | No |
| `--help` | `-h` | Show help message and exit | No |
| `--version` | `-v` | Show version (build date) and exit | No |

The tool will provide verbose error messages for any issues encountered during conversion.

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

## Testing

The project includes comprehensive unit tests using Google Test framework. To build and run tests:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

### Test Structure

- **Unit Tests**: Test individual components in isolation
  - `FileReaderTest`: Tests file reading with mmap support
  - `YamlToJsonConverterTest`: Tests YAML to JSON conversion
  - `JsonFormatterTest`: Tests JSON pretty-printing
  - `ErrorHandlerTest`: Tests error handling and reporting
- **Integration Tests**: Test the complete workflow
  - End-to-end conversion tests
  - Error handling scenarios
  - Performance tests with large files

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for complete terms.

## Contributing

Contributions are accepted following the guidelines in [CONTRIBUTING.md](CONTRIBUTING.md). 