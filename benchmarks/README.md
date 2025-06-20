# Benchmarks

Professional performance benchmarks for yaml2json using [hyperfine](https://github.com/sharkdp/hyperfine).

## Quick Start

```bash
# Generate test files
./generate_compatible_yaml.sh

# Run all benchmarks
./benchmark.sh
```

## Results

The benchmarks compare yaml2json against yq and lq on files of different sizes:

| File Size | yaml2json | yq | lq | Speedup vs yq | Speedup vs lq |
|-----------|-----------|----|----|---------------|---------------|
| **117KB** | **2.1ms ± 0.3ms** | 22.9ms ± 0.9ms | 20.0ms ± 0.2ms | **10.7x faster** | **9.4x faster** |
| **0.99MB** | **12.5ms ± 0.6ms** | 169.4ms ± 5.1ms | 145.5ms ± 0.9ms | **13.5x faster** | **11.6x faster** |
| **6.53MB** | **68.8ms ± 1.2ms** | 1.012s ± 0.042s | 911.8ms ± 48.5ms | **14.7x faster** | **13.3x faster** |
| **13.23MB** | **139.8ms ± 1.1ms** | 2.064s ± 0.053s | 1.857s ± 0.042s | **14.8x faster** | **13.3x faster** |

## Files

- `benchmark.sh` - Main benchmarking script using hyperfine
- `generate_compatible_yaml.sh` - Creates test YAML files
- `*_results.json` - Hyperfine results in JSON format
- `*_results.md` - Hyperfine results in Markdown format
- `*.yaml` - Generated test files

## Requirements

- [hyperfine](https://github.com/sharkdp/hyperfine) - `brew install hyperfine`
- [yq](https://github.com/mikefarah/yq) - `brew install yq`
- [lq](https://github.com/jzelinskie/lq) - Download from releases

## Methodology

- **Tool**: hyperfine for professional benchmarking
- **Warmup**: 3 runs to eliminate cold start effects
- **Measurements**: 10 runs for statistical accuracy
- **Output**: JSON and Markdown formats for analysis
- **Reproducibility**: All test files and scripts included

## Test Files

The benchmark uses four test files with different characteristics:

1. **small_117kb.yaml** (117KB) - Small dataset for baseline testing
2. **medium_1mb.yaml** (0.99MB) - Moderate complexity
3. **large_6_5mb.yaml** (6.53MB) - Large structured data
4. **very_large_13mb.yaml** (13.23MB) - Very large datasets

All files are generated to be compatible with all three tools (no advanced YAML features that might cause parsing differences).

## Reproducing Results

```bash
# Install dependencies
brew install hyperfine yq bc coreutils

# Download lq
wget https://github.com/jzelinskie/lq/releases/latest/download/lq-aarch64-apple-darwin
mv lq-aarch64-apple-darwin lq
chmod +x lq

# Generate test files
./generate_compatible_yaml.sh

# Run benchmarks
./benchmark.sh
```

The results will be saved as JSON and Markdown files for further analysis. 