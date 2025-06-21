# yaml2json

A YAML to JSON converter implemented using the RapidYAML library, designed for high-throughput data processing applications.

## Performance Analysis

Comparative performance measurements were conducted using [hyperfine](https://github.com/sharkdp/hyperfine) on Apple Silicon (macOS) against established tools yq and lq:

| File Size | yaml2json | yq | lq | Relative Performance |
|-----------|-----------|----|----|---------------------|
| 117KB | **3.0ms ± 0.4ms** | 23.4ms ± 1.0ms | 20.6ms ± 0.4ms | **7.9x / 6.95x improvement** |
| 0.99MB | **8.9ms ± 0.2ms** | 171.7ms ± 10.5ms | 147.5ms ± 5.3ms | **19.35x / 16.63x improvement** |
| 6.53MB | **47.9ms ± 0.4ms** | 991.6ms ± 22.1ms | 905.7ms ± 23.5ms | **20.69x / 18.90x improvement** |
| 13.23MB | **96.4ms ± 2.2ms** | 2.091s ± 0.061s | 1.871s ± 0.052s | **21.68x / 19.40x improvement** |

### Implementation Characteristics

- Zero-copy parsing using memory-mapped file input
- Optimized compiler flags: `-O3 -march=native` with Link Time Optimization (LTO)
- Direct JSON serialization without intermediate data structures
- Linear time complexity scaling with input size
- Deterministic parsing behavior across test cases
- Excellent performance scaling (**7-21x improvement** over competitors, with **better scaling** on larger files)

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

The tool supports multiple input/output methods for maximum flexibility and backwards compatibility:

### File-to-File Conversion

```bash
# Explicit flags (recommended for scripts)
yaml2json --input input.yaml --output output.json
yaml2json -i input.yaml -o output.json

# Positional arguments (backwards compatible)
yaml2json input.yaml output.json
yaml2json input.yaml output.json --pretty
```

### Pipeline/Stream Processing

```bash
# Stdin to stdout (great for pipelines)
cat input.yaml | yaml2json
curl -s https://example.com/data.yaml | yaml2json

# File to stdout
yaml2json input.yaml
yaml2json --input input.yaml

# Stdin to file
cat input.yaml | yaml2json --output output.json
```

### Pretty-Print JSON

```bash
# Any of the above with pretty formatting
yaml2json input.yaml --pretty
yaml2json -i input.yaml -o output.json --pretty
cat input.yaml | yaml2json --pretty
```

### Command-Line Options

| Option | Short | Description | Required |
|--------|-------|-------------|----------|
| `--input` | `-i` | Input YAML file path | No* |
| `--output` | `-o` | Output JSON file path | No* |
| `--pretty` | `-p` | Pretty-print JSON with indentation | No |
| `--help` | `-h` | Show help message and exit | No |
| `--version` | `-v` | Show version (build date) and exit | No |

\* *Uses stdin/stdout when not specified. Supports positional arguments for backwards compatibility.*

### Usage Examples

```bash
# Traditional usage
yaml2json config.yaml config.json

# Pipeline processing
kubectl get pods -o yaml | yaml2json | jq '.items[0].metadata.name'

# Mixed usage
yaml2json --input config.yaml | jq '.database.host'
cat config.yaml | yaml2json --output config.json --pretty
```

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