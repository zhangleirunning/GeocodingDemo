#!/bin/bash
# Script to format all C++ code according to Google C++ Style Guide

set -e

echo "Formatting C++ code with clang-format..."

# Find all .cpp and .h files and format them
find include src test -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i -style=file {} \;

echo "Code formatting complete!"
echo ""
echo "To check for style violations, run:"
echo "  ./scripts/check_style.sh"
