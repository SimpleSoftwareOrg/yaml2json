# Benchmarks

Professional performance benchmarks for yaml2json using [hyperfine](https://github.com/sharkdp/hyperfine).

## Quick Start

```bash
# Run benchmarks (everything is automated)
./benchmark.sh
```

The script automatically:
- Downloads lq binary if not present
- Generates test files if needed
- Runs comprehensive benchmarks
- Saves results in JSON and Markdown formats

## Results

The benchmarks compare yaml2json against yq and lq on files of different sizes:

| File Size | yaml2json | yq | lq | Speedup vs yq | Speedup vs lq |
|-----------|-----------|----|----|---------------|---------------|
| **117KB** | **3.1ms ± 0.7ms** | 22.8ms ± 0.5ms | 20.4ms ± 0.4ms | **7.3x faster** | **6.6x faster** |
| **0.99MB** | **12.2ms ± 1.0ms** | 168.9ms ± 2.9ms | 150.0ms ± 20.8ms | **13.8x faster** | **12.3x faster** |
| **6.53MB** | **68.4ms ± 0.4ms** | 992.0ms ± 45.4ms | 898.4ms ± 25.1ms | **14.5x faster** | **13.1x faster** |
| **13.23MB** | **140.4ms ± 2.7ms** | 2.078s ± 0.063s | 1.856s ± 0.048s | **14.8x faster** | **13.2x faster** |

## Files

- `benchmark.sh` - Main benchmarking script (auto-downloads dependencies, generates test files)
- `generate_compatible_yaml.sh` - Creates test YAML files (called automatically)
- `*_results.json` - Hyperfine results in JSON format (generated)
- `*_results.md` - Hyperfine results in Markdown format (generated)
- `*.yaml` - Generated test files (generated)
- `lq` - Downloaded lq binary (downloaded automatically)

## Requirements

### Required
- [hyperfine](https://github.com/sharkdp/hyperfine) - `brew install hyperfine`
- [yq](https://github.com/mikefarah/yq) - `brew install yq`
- `bc` - `brew install bc` (for file size calculations)

### Automatic (downloaded by script)
- [lq](https://github.com/jzelinskie/lq) - Downloaded automatically for your platform

## Methodology

- **Tool**: hyperfine for professional benchmarking
- **Warmup**: 3 runs to eliminate cold start effects
- **Measurements**: 20 runs for statistical accuracy
- **Output**: JSON and Markdown formats for analysis
- **Reproducibility**: All dependencies handled automatically

## Test Files

The benchmark uses four test files with different characteristics:

1. **small_117kb.yaml** (117KB) - Small dataset for baseline testing
2. **medium_1mb.yaml** (0.99MB) - Moderate complexity
3. **large_6_5mb.yaml** (6.53MB) - Large structured data
4. **very_large_13mb.yaml** (13.23MB) - Very large datasets

All files are generated to be compatible with all three tools (no advanced YAML features that might cause parsing differences).

## Cross-Platform Support

The benchmark script automatically detects your platform and downloads the appropriate lq binary:

- **macOS**: Both Intel (x86_64) and Apple Silicon (aarch64)
- **Linux**: Both x86_64 and ARM64 (aarch64) architectures

## Reproducing Results

```bash
# Install only the required dependencies  
brew install hyperfine yq bc

# Run benchmarks (everything else is automatic)
./benchmark.sh
```

The script handles all other dependencies and test data generation automatically. Results are saved as JSON and Markdown files for further analysis. 