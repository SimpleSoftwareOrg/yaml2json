#!/bin/bash

set -e

# Default installation directory
INSTALL_DIR="/usr/local/bin"
USER_INSTALL=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --user)
            USER_INSTALL=true
            INSTALL_DIR="$HOME/bin"
            shift
            ;;
        --dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --user      Install to ~/bin instead of /usr/local/bin"
            echo "  --dir DIR   Install to custom directory"
            echo "  -h, --help  Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Detect platform and architecture
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

case $OS in
    linux)
        PLATFORM="linux"
        ;;
    darwin)
        PLATFORM="macos"
        ;;
    mingw*|cygwin*|msys*)
        PLATFORM="windows"
        ;;
    *)
        echo "Unsupported operating system: $OS"
        exit 1
        ;;
esac

case $ARCH in
    x86_64|amd64)
        ARCH="x86_64"
        ;;
    arm64|aarch64)
        ARCH="arm64"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

# Construct binary name
BINARY_NAME="yaml2json-${PLATFORM}-${ARCH}"
if [[ $PLATFORM == "windows" ]]; then
    BINARY_NAME="${BINARY_NAME}.exe"
    TARGET_NAME="yaml2json.exe"
else
    TARGET_NAME="yaml2json"
fi

# GitHub repository (update this to your actual repository)
REPO="SimpleSoftwareOrg/yaml2json"
LATEST_RELEASE_URL="https://api.github.com/repos/${REPO}/releases/latest"

echo "Installing yaml2json for ${PLATFORM}-${ARCH}..."

# Get latest release information
echo "Fetching latest release information..."
RELEASE_INFO=$(curl -s "$LATEST_RELEASE_URL")
DOWNLOAD_URL=$(echo "$RELEASE_INFO" | grep "browser_download_url.*${BINARY_NAME}" | cut -d '"' -f 4)

if [[ -z "$DOWNLOAD_URL" ]]; then
    echo "Error: Could not find download URL for ${BINARY_NAME}"
    echo "Available assets:"
    echo "$RELEASE_INFO" | grep "browser_download_url" | cut -d '"' -f 4
    exit 1
fi

echo "Download URL: $DOWNLOAD_URL"

# Create installation directory if it doesn't exist
if [[ ! -d "$INSTALL_DIR" ]]; then
    if [[ $USER_INSTALL == true ]]; then
        mkdir -p "$INSTALL_DIR"
    else
        echo "Creating directory $INSTALL_DIR (may require sudo)..."
        sudo mkdir -p "$INSTALL_DIR"
    fi
fi

# Download and install
TEMP_FILE=$(mktemp)
echo "Downloading yaml2json..."
curl -L -o "$TEMP_FILE" "$DOWNLOAD_URL"

# Verify download
if [[ ! -s "$TEMP_FILE" ]]; then
    echo "Error: Download failed or file is empty"
    exit 1
fi

# Install the binary
TARGET_PATH="$INSTALL_DIR/$TARGET_NAME"
if [[ $USER_INSTALL == true ]]; then
    cp "$TEMP_FILE" "$TARGET_PATH"
    chmod +x "$TARGET_PATH"
else
    echo "Installing to $TARGET_PATH (may require sudo)..."
    sudo cp "$TEMP_FILE" "$TARGET_PATH"
    sudo chmod +x "$TARGET_PATH"
fi

# Cleanup
rm "$TEMP_FILE"

echo "yaml2json installed successfully to $TARGET_PATH"

# Test installation
if command -v "$TARGET_NAME" &> /dev/null; then
    echo "Testing installation..."
    if echo "test: value" | "$TARGET_NAME" | grep -q '{"test":"value"}'; then
        echo "✅ Installation verified successfully!"
    else
        echo "⚠️  Installation completed but test failed"
    fi
else
    echo "⚠️  Installation completed but $TARGET_NAME is not in PATH"
    if [[ $USER_INSTALL == true ]]; then
        echo "   Add $INSTALL_DIR to your PATH to use yaml2json from anywhere"
        echo "   Add this to your shell profile:"
        echo "   export PATH=\"$INSTALL_DIR:\$PATH\""
    fi
fi 