#!/bin/bash

set -e

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <path-to-build-dir>"
  echo "Example: ./scripts/run_linters.sh build-clang-cpp17"
  exit 1
fi

BUILD_DIR="$1"

# Get absolute path to script and move to repo root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Generate compile_commands.json if not found
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
  echo "Generating compile_commands.json in $BUILD_DIR..."
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B "$BUILD_DIR"
else
  echo "Using existing $BUILD_DIR/compile_commands.json"
fi

# Run clang-format
echo "Running clang-format..."
find . -type f \( -name '*.cpp' -o -name '*.hpp' \) -exec clang-format -i {} +

# Run clang-tidy with auto-fix
echo "Running clang-tidy with auto-fix..."
find . -type f \( -name '*.cpp' -o -name '*.hpp' \) -exec clang-tidy -p "$BUILD_DIR" -fix -fix-errors {} +

echo "✅ Linting complete."
