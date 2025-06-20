#!/bin/bash

set -e

# Professional benchmarking using hyperfine
# This script provides trusted performance measurements for yaml2json vs yq vs lq

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
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

print_warning() {
    echo -e "${YELLOW}$1${NC}"
}

# Download lq if not present
download_lq() {
    if [[ -f "lq" ]]; then
        print_success "✓ lq: $(realpath lq)"
        return
    fi
    
    print_warning "⬇ Downloading lq binary..."
    
    # Detect OS and architecture
    local os=$(uname -s | tr '[:upper:]' '[:lower:]')
    local arch=$(uname -m)
    
    # Map architecture names
    case $arch in
        x86_64) arch="x86_64" ;;
        arm64|aarch64) arch="aarch64" ;;
        *) echo "❌ Unsupported architecture: $arch"; exit 1 ;;
    esac
    
    # Map OS names for clux/lq naming convention
    case $os in
        darwin) os="apple-darwin" ;;
        linux) os="unknown-linux-gnu" ;;
        *) echo "❌ Unsupported OS: $os"; exit 1 ;;
    esac
    
    local archive_name="lq-${arch}-${os}.tar.xz"
    local download_url="https://github.com/clux/lq/releases/latest/download/${archive_name}"
    
    if command -v curl &> /dev/null; then
        curl -L -o "$archive_name" "$download_url"
    elif command -v wget &> /dev/null; then
        wget -O "$archive_name" "$download_url"
    else
        echo "❌ Neither curl nor wget found. Please install one to download lq."
        exit 1
    fi
    
    # Extract the binary from tar.xz
    if command -v tar &> /dev/null; then
        tar -xf "$archive_name" --strip-components=1 "*/lq" 2>/dev/null || tar -xf "$archive_name" "lq" 2>/dev/null
        rm "$archive_name"
    else
        echo "❌ tar not found. Cannot extract lq binary."
        exit 1
    fi
    
    chmod +x lq
    print_success "✓ lq downloaded and ready"
}

# Generate test files if they don't exist
generate_test_files() {
    local test_files=("small_117kb.yaml" "medium_1mb.yaml" "large_6_5mb.yaml" "very_large_13mb.yaml")
    local missing_files=()
    
    # Check which files are missing
    for file in "${test_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            missing_files+=("$file")
        fi
    done
    
    # Generate missing files if any
    if [[ ${#missing_files[@]} -gt 0 ]]; then
        print_warning "⚡ Generating test files..."
        ./generate_compatible_yaml.sh > /dev/null 2>&1
        print_success "✓ Test files generated"
    else
        print_success "✓ Test files already exist"
    fi
}

# Check if tools are available
check_tools() {
    print_header "Setup and Dependencies"
    
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
    
    # Download lq if needed
    download_lq
    
    # Generate test files if needed
    generate_test_files
    
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
    print_info "Using 3 warmup runs and 20 measurement runs"
    echo ""
    
    # Run hyperfine benchmark
    hyperfine -N --warmup 3 --runs 20 \
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
    
    # Define test files and names
    local test_files=("small_117kb.yaml" "medium_1mb.yaml" "large_6_5mb.yaml" "very_large_13mb.yaml")
    local test_names=("small" "medium" "large" "very_large")
    
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