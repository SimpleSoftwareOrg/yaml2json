# Contributing to yaml2json

Thank you for your interest in contributing to yaml2json! This document provides guidelines for contributing to the project.

## Development Setup

### Prerequisites

- **C++17 compatible compiler**:
  - GCC 8+ on Linux
  - Clang 8+ on macOS/Linux  
  - MSVC 2019+ on Windows
- **CMake 3.15+**
- **Git**

### Building from Source

```bash
git clone https://github.com/SimpleSoftwareOrg/yaml2json.git
cd yaml2json
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)  # or cmake --build . --parallel
```

### Testing

Test your build with a simple conversion:

```bash
echo "test: value" | ./yaml2json
# Expected output: {"test":"value"}
```

## Making Changes

### Code Style

- Use **4 spaces** for indentation (no tabs)
- Follow existing naming conventions
- Keep lines under **100 characters** where possible
- Use modern C++17 features appropriately

### Performance Considerations

This project prioritizes performance. When making changes:

- **Profile before optimizing** - use benchmarks to verify improvements
- **Preserve zero-copy parsing** where possible
- **Test with large files** (>10MB) to ensure scalability

### Commit Messages

Use clear, descriptive commit messages:

```
feat: add support for custom output formatting
fix: handle edge case with empty YAML documents  
perf: optimize memory allocation for large files
docs: update installation instructions
```

## Submitting Changes

### Pull Request Process

1. **Fork** the repository
2. **Create a feature branch** from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes** with appropriate tests
4. **Ensure the build passes**:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```
5. **Test functionality**:
   ```bash
   echo "test: value" | ./yaml2json
   ```
6. **Submit a pull request** with:
   - Clear description of changes
   - Performance impact (if applicable)
   - Test results

### Pull Request Guidelines

- **One feature per PR** - keep changes focused
- **Include tests** for new functionality
- **Update documentation** if needed
- **Ensure CI passes** - all platforms must build successfully

## Performance Testing

To run performance benchmarks:

```bash
# Create test files (or use your own)
echo "large_data: $(seq 1 10000)" > large_test.yaml

# Time the conversion
time ./yaml2json large_test.yaml > /dev/null

# Compare with other tools
time yq -o json large_test.yaml > /dev/null
time lq large_test.yaml > /dev/null
```

## Reporting Issues

### Bug Reports

When reporting bugs, include:

- **Operating system** and version
- **Compiler** and version used
- **Input YAML** that causes the issue (minimal reproduction case)
- **Expected behavior** vs **actual behavior**
- **Error messages** or crash logs

### Feature Requests

For feature requests:

- **Describe the use case** clearly
- **Explain why** the feature would be valuable
- **Consider performance impact** - will it slow down existing functionality?
- **Suggest implementation** if you have ideas

## Release Process

Releases are automated via GitHub Actions:

1. **Create a tag** following semantic versioning:
   ```bash
   git tag v1.2.3
   git push origin v1.2.3
   ```

2. **GitHub Actions** automatically:
   - Builds binaries for all supported platforms
   - Runs tests
   - Creates a GitHub release with binaries
   - Updates package managers

## Getting Help

- **GitHub Issues** - for bugs and feature requests
- **GitHub Discussions** - for questions and general discussion
- **Code Review** - maintainers will provide feedback on PRs

## License

By contributing to yaml2json, you agree that your contributions will be licensed under the MIT License. 