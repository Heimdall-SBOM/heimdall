#!/bin/bash

# Heimdall Dependency Installer
# This script installs the required dependencies for building Heimdall

set -e

echo "=== Heimdall Dependency Installer ==="

# Detect OS
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Detected macOS"
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. Please install Homebrew first:"
        echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    echo "Installing dependencies via Homebrew..."
    
    # Install LLVM (includes clang, lld, etc.)
    echo "Installing LLVM..."
    brew install llvm
    
    # Install LLD separately (in case it's not included in LLVM)
    echo "Installing LLD..."
    brew install lld
    
    # Install OpenSSL
    echo "Installing OpenSSL..."
    brew install openssl@3
    
    # Install CMake if not already installed
    if ! command -v cmake &> /dev/null; then
        echo "Installing CMake..."
        brew install cmake
    fi
    
    # Install Ninja for faster builds (optional)
    if ! command -v ninja &> /dev/null; then
        echo "Installing Ninja (optional, for faster builds)..."
        brew install ninja
    fi
    
    echo ""
    echo "=== Installation Complete ==="
    echo "Dependencies installed:"
    echo "  - LLVM (includes clang, lld)"
    echo "  - OpenSSL 3.x"
    echo "  - CMake"
    echo "  - Ninja (optional)"
    echo ""
    echo "You can now build Heimdall with:"
    echo "  ./scripts/build.sh --standard 17 --compiler clang --clean --tests"
    
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Detected Linux"
    
    # Detect distribution
    if command -v apt-get &> /dev/null; then
        echo "Detected Debian/Ubuntu-based system"
        echo "Installing dependencies via apt..."
        
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            libssl-dev \
            llvm-dev \
            clang \
            lld \
            libc++-dev \
            libc++abi-dev
        
    elif command -v dnf &> /dev/null; then
        echo "Detected Fedora/RHEL-based system"
        echo "Installing dependencies via dnf..."
        
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            ninja-build \
            openssl-devel \
            llvm-devel \
            clang \
            lld \
            libcxx-devel \
            libcxxabi-devel
            
    elif command -v pacman &> /dev/null; then
        echo "Detected Arch Linux"
        echo "Installing dependencies via pacman..."
        
        sudo pacman -S --needed \
            base-devel \
            cmake \
            ninja \
            openssl \
            llvm \
            clang \
            lld \
            libc++
            
    else
        echo "Unsupported Linux distribution. Please install the following manually:"
        echo "  - build-essential (or equivalent)"
        echo "  - cmake"
        echo "  - ninja-build (optional)"
        echo "  - libssl-dev"
        echo "  - llvm-dev"
        echo "  - clang"
        echo "  - lld"
        echo "  - libc++-dev"
        echo "  - libc++abi-dev"
        exit 1
    fi
    
    echo ""
    echo "=== Installation Complete ==="
    echo "Dependencies installed. You can now build Heimdall with:"
    echo "  ./scripts/build.sh --standard 17 --compiler clang --clean --tests"
    
else
    echo "Unsupported operating system: $OSTYPE"
    echo "Please install the following dependencies manually:"
    echo "  - LLVM (with clang and lld)"
    echo "  - OpenSSL 3.x"
    echo "  - CMake"
    echo "  - Ninja (optional)"
    exit 1
fi 