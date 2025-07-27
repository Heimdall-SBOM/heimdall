#!/bin/bash

set -e

usage() {
  echo "Usage: $0 <build-dir> [--dry-run]"
  echo "  --dry-run     Check formatting and tidy issues, but do not apply fixes"
  echo "Example:"
  echo "  $0 build-clang-cpp17            # Apply formatting and fixes"
  echo "  $0 build-clang-cpp17 --dry-run  # Only check, do not modify files"
}

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
  usage
  exit 1
fi

BUILD_DIR="$1"
DRY_RUN=false

if [ "$2" == "--dry-run" ]; then
  DRY_RUN=true
elif [ -n "$2" ]; then
  echo "Unknown option: $2"
  usage
  exit 1
fi

# Determine script and repo root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

# Lint targets
LINT_DIRS=(examples src tests tools)

# Ensure build dir exists and compile_commands.json is present
mkdir -p "$BUILD_DIR"
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
  echo "Generating compile_commands.json in $BUILD_DIR..."
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B "$BUILD_DIR"
else
  echo "Using existing $BUILD_DIR/compile_commands.json"
fi

echo "Linting target directories: ${LINT_DIRS[*]}"
echo "Dry run mode: $DRY_RUN"
echo

# clang-format
echo "Running clang-format..."
for dir in "${LINT_DIRS[@]}"; do
  if [ -d "$dir" ]; then
    find "$dir" -type f \( -name '*.cpp' -o -name '*.hpp' \) | while read -r file; do
      if $DRY_RUN; then
        if ! diff -u "$file" <(clang-format "$file") >/dev/null; then
          echo "clang-format violation in $file"
          diff -u "$file" <(clang-format "$file") || true
        fi
      else
        clang-format -i "$file"
      fi
    done
  fi
done

# clang-tidy
echo
echo "Running clang-tidy..."
HEADER_FILTER='^(examples|src|tests|tools)/'
for dir in "${LINT_DIRS[@]}"; do
  if [ -d "$dir" ]; then
    find "$dir" -type f \( -name '*.cpp' -o -name '*.hpp' \) | while read -r file; do
      if $DRY_RUN; then
        echo "Checking $file..."
        clang-tidy -p "$BUILD_DIR" --header-filter="$HEADER_FILTER" "$file" || true
      else
        echo "Fixing $file..."
        clang-tidy -p "$BUILD_DIR" --header-filter="$HEADER_FILTER" -fix -fix-errors "$file" || true
      fi
    done
  fi
done

echo
echo "✅ Linting complete."
