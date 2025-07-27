#!/bin/bash

# SCL Compiler Wrapper
# This script wraps SCL-enabled compiler commands for use with CMake

if [ $# -eq 0 ]; then
    echo "Usage: $0 <scl-toolset> <compiler> [args...]"
    echo "Example: $0 gcc-toolset-14 gcc -v"
    exit 1
fi

SCL_TOOLSET="$1"
COMPILER="$2"
shift 2

# Execute the compiler with SCL environment
exec scl enable "$SCL_TOOLSET" -- "$COMPILER" "$@" 