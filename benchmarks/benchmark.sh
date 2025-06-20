#!/bin/bash

set -e

# Professional benchmarking using hyperfine
# This script provides trusted performance measurements for yaml2json vs yq vs lq

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_info() {
    echo -e "${CYAN}$1${NC}"
}

print_success() {
    echo -e "${GREEN}$1${NC}"
}

# Check if tools are available
check_tools() {
    print_header "Checking Tools"
    
    # Check hyperfine
    if ! command -v hyperfine &> /dev/null; then
        echo "❌ hyperfine not found. Install with: brew install hyperfine"
        exit 1
    fi
    print_success "✓ hyperfine: $(which hyperfine)"
    
    # Check yaml2json
    if [[ ! -f "../build/yaml2json" ]]; then
        echo "❌ yaml2json not found. Please build it first."
        exit 1
    fi
    print_success "✓ yaml2json: $(realpath ../build/yaml2json)"
    
    # Check yq
    if ! command -v yq &> /dev/null; then
        echo "❌ yq not found. Install with: brew install yq"
        exit 1
    fi
    print_success "✓ yq: $(which yq)"
    
    # Check lq
    if [[ ! -f "lq" ]]; then
        echo "❌ lq not found in benchmarks/"
        exit 1
    fi
    print_success "✓ lq: $(realpath lq)"
    
    echo ""
}

# Run benchmark for a specific file
benchmark_file() {
    local file=$1
    local name=$2
    local filesize=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file")
    local filesize_mb=$(echo "scale=2; $filesize / 1048576" | bc -l)
    
    print_header "Benchmarking: $name (${filesize_mb}MB)"
    print_info "File: $file"
    print_info "Using 3 warmup runs and 10 measurement runs"
    echo ""
    
    # Run hyperfine benchmark
    hyperfine --warmup 3 --runs 10 \
        --export-json "${name}_results.json" \
        --export-markdown "${name}_results.md" \
        "../build/yaml2json $file" \
        "yq -o json $file" \
        "./lq . $file"
    
    echo ""
    print_success "✓ Results saved to ${name}_results.json and ${name}_results.md"
    echo ""
}

# Main execution
main() {
    print_header "YAML to JSON Performance Benchmarks"
    print_info "Professional benchmarking using hyperfine"
    print_info "This provides trusted, reproducible performance measurements"
    echo ""
    
    check_tools
    
    # Check if test files exist
    local test_files=("small_117kb.yaml" "medium_1mb.yaml" "large_6_5mb.yaml" "very_large_13mb.yaml")
    local test_names=("small" "medium" "large" "very_large")
    
    for i in "${!test_files[@]}"; do
        local file="${test_files[$i]}"
        local name="${test_names[$i]}"
        
        if [[ ! -f "$file" ]]; then
            echo "❌ Test file $file not found. Please run generate_compatible_yaml.sh first."
            exit 1
        fi
    done
    
    # Run benchmarks
    for i in "${!test_files[@]}"; do
        benchmark_file "${test_files[$i]}" "${test_names[$i]}"
    done
    
    print_header "Benchmark Complete"
    print_success "All benchmark results have been saved with JSON and Markdown formats"
    print_info "JSON files can be used for further analysis"
    print_info "Markdown files provide human-readable summaries"
    echo ""
    print_info "To reproduce these results, run: ./benchmark.sh"
}

# Ensure bc is available for file size calculations
if ! command -v bc &> /dev/null; then
    echo "bc not found. Install with: brew install bc"
    exit 1
fi

main "$@" 