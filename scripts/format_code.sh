#!/bin/bash
# Script to format all C++ code according to Google C++ Style Guide
#
# Requirements:
#   - clang-format (install: brew install llvm)
#
# Usage:
#   ./scripts/format_code.sh

set -e

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "❌ Error: clang-format is not installed"
    echo ""
    echo "Install with:"
    echo "  macOS:  brew install llvm"
    echo "  Linux:  apt-get install clang-format"
    echo ""
    exit 1
fi

echo "Formatting C++ code with clang-format..."

# Find all .cpp and .h files and format them
find include src test -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i -style=file {} \;

echo "Code formatting complete!"
echo ""
echo "To check for style violations, run:"
echo "  ./scripts/check_style.sh"
