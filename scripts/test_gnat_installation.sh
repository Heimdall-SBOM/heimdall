#!/bin/bash

# Test script to verify GNAT installation in Docker containers
# This script can be run inside any of the Heimdall Docker containers

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_status "Testing GNAT installation in Docker container..."

# Test 1: Check if GNAT is available
print_status "Test 1: Checking GNAT availability..."
if command -v gnatmake >/dev/null 2>&1; then
    print_success "GNAT found: $(gnatmake --version | head -n1)"
else
    print_error "GNAT not found in PATH"
    exit 1
fi

# Test 2: Check if gnatgcc is available
print_status "Test 2: Checking gnatgcc availability..."
if command -v gnatgcc >/dev/null 2>&1; then
    print_success "gnatgcc found: $(gnatgcc --version | head -n1)"
else
    print_error "gnatgcc not found in PATH"
    exit 1
fi

# Test 3: Check if gnatbind is available
print_status "Test 3: Checking gnatbind availability..."
if command -v gnatbind >/dev/null 2>&1; then
    print_success "gnatbind found"
else
    print_error "gnatbind not found in PATH"
    exit 1
fi

# Test 4: Check if gnatlink is available
print_status "Test 4: Checking gnatlink availability..."
if command -v gnatlink >/dev/null 2>&1; then
    print_success "gnatlink found"
else
    print_error "gnatlink not found in PATH"
    exit 1
fi

# Test 5: Create a simple Ada test program
print_status "Test 5: Creating and compiling a simple Ada test program..."

cat > test_gnat.adb << 'EOF'
with Ada.Text_IO; use Ada.Text_IO;

procedure Test_GNAT is
begin
   Put_Line("GNAT is working correctly!");
   Put_Line("Ada compiler is properly installed and functional.");
end Test_GNAT;
EOF

# Compile the test program
if gnatmake test_gnat.adb; then
    print_success "Ada test program compiled successfully"
    
    # Run the test program
    if ./test_gnat; then
        print_success "Ada test program ran successfully"
    else
        print_error "Ada test program failed to run"
        exit 1
    fi
else
    print_error "Failed to compile Ada test program"
    exit 1
fi

# Clean up
rm -f test_gnat test_gnat.ali test_gnat.o test_gnat.adb

print_success "All GNAT tests passed! GNAT is properly installed and working."
print_status "This container is ready to build the Heimdall Ada demo examples." 