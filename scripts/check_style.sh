#!/bin/bash
# Script to check C++ code style compliance

set -e

echo "Checking C++ code style with clang-format..."

# Check if any files would be reformatted
NEEDS_FORMAT=$(find include src test -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -style=file --dry-run --Werror {} \; 2>&1 || true)

if [ -n "$NEEDS_FORMAT" ]; then
    echo "❌ Style violations found:"
    echo "$NEEDS_FORMAT"
    echo ""
    echo "Run './scripts/format_code.sh' to fix formatting issues."
    exit 1
else
    echo "✅ All files are properly formatted!"
fi

echo ""
echo "Running clang-tidy checks..."

# Run clang-tidy on all source files
find src test -type f -name "*.cpp" -exec clang-tidy {} -- -I./include -std=c++17 \;

echo "✅ Style checks complete!"
