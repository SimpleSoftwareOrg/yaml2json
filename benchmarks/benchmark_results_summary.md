# yaml2json Performance Benchmark Results - Current Branch

**Benchmark Date:** June 21, 2024  
**Environment:** Linux, AWS Instance  
**Tools Compared:** yaml2json vs yq (v4.45.4) vs lq

## Summary

yaml2json demonstrates **excellent performance** across all file sizes, consistently outperforming both yq and lq by significant margins.

## Detailed Results

### Small File (117KB)
| Tool | Time (mean ± σ) | Speedup vs yaml2json |
|------|----------------|---------------------|
| **yaml2json** | **6.1 ms ± 0.2 ms** | 1.0x (baseline) |
| lq | 15.5 ms ± 0.5 ms | 2.52x slower |
| yq | 32.0 ms ± 0.9 ms | 5.22x slower |

### Medium File (1MB)
| Tool | Time (mean ± σ) | Speedup vs yaml2json |
|------|----------------|---------------------|
| **yaml2json** | **42.2 ms ± 0.8 ms** | 1.0x (baseline) |
| lq | 120.9 ms ± 2.5 ms | 2.86x slower |
| yq | 263.9 ms ± 4.0 ms | 6.25x slower |

### Large File (6.5MB)
| Tool | Time (mean ± σ) | Speedup vs yaml2json |
|------|----------------|---------------------|
| **yaml2json** | **270.9 ms ± 2.6 ms** | 1.0x (baseline) |
| lq | 753.0 ms ± 8.5 ms | 2.78x slower |
| yq | 1.556 s ± 0.014 s | 5.74x slower |

### Very Large File (13.2MB)
| Tool | Time (mean ± σ) | Speedup vs yaml2json |
|------|----------------|---------------------|
| **yaml2json** | **577.1 ms ± 8.4 ms** | 1.0x (baseline) |
| lq | 1.548 s ± 0.012 s | 2.68x slower |
| yq | 3.210 s ± 0.025 s | 5.56x slower |

## Performance Analysis

### Key Findings:
1. **Consistent Speed Advantage**: yaml2json maintains 2.5-3x advantage over lq and 5-6x advantage over yq across all file sizes
2. **Excellent Scalability**: Performance scales linearly with file size, showing no degradation at larger sizes
3. **Low Variance**: Very consistent performance with low standard deviation
4. **Memory Efficiency**: System memory usage remains reasonable even for large files

### Performance Characteristics:
- **Small files**: yaml2json excels with sub-10ms processing times
- **Medium files**: Maintains strong performance at ~42ms for 1MB files
- **Large files**: Scales well to ~271ms for 6.5MB files
- **Very large files**: Still very competitive at ~577ms for 13.2MB files

## Benchmark Methodology

- **Tool**: hyperfine (professional benchmarking)
- **Warmup runs**: 3 (to eliminate cold start effects)
- **Measurement runs**: 20 (for statistical accuracy)
- **Output**: JSON and Markdown formats for analysis
- **Test files**: Compatible YAML generated to work with all tools

## Conclusion

The current yaml2json implementation shows **excellent performance characteristics** with:
- ✅ **5-6x faster than yq** (industry standard)
- ✅ **2.5-3x faster than lq** (performance-focused alternative)
- ✅ **Consistent performance** across all file sizes
- ✅ **Linear scalability** with file size
- ✅ **Low variance** indicating stable performance

The performance is very competitive and would satisfy most production use cases requiring high-speed YAML to JSON conversion.