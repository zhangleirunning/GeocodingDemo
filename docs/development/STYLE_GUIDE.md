# Google C++ Style Guide Enforcement

This project follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## Tools

We use the following tools to enforce code style:

1. **clang-format**: Automatic code formatting
2. **clang-tidy**: Static analysis and style checking
3. **pre-commit**: Git hooks for automatic checking

## Installation

### macOS

```bash
# Install clang-format and clang-tidy
brew install clang-format llvm

# Install pre-commit (optional but recommended)
pip install pre-commit
pre-commit install
```

### Linux (Ubuntu/Debian)

```bash
# Install clang-format and clang-tidy
sudo apt-get install clang-format clang-tidy

# Install pre-commit (optional but recommended)
pip install pre-commit
pre-commit install
```

## Usage

### Format Code Automatically

```bash
# Using CMake target
cmake --build build --target format

# Or using the script
./scripts/format_code.sh

# Or format a single file
clang-format -i -style=file path/to/file.cpp
```

### Check Code Style

```bash
# Using CMake target
cmake --build build --target check-format

# Or using the script
./scripts/check_style.sh

# Or check a single file
clang-format -style=file --dry-run --Werror path/to/file.cpp
```

### Run Static Analysis

```bash
# Run clang-tidy on all files
find src test -name "*.cpp" -exec clang-tidy {} -- -I./include -std=c++17 \;

# Or on a single file
clang-tidy path/to/file.cpp -- -I./include -std=c++17
```

## IDE Integration

### VS Code

Install the following extensions:
- **C/C++** (Microsoft)
- **clangd** (LLVM)
- **Clang-Format** (xaver)

Add to `.vscode/settings.json`:

```json
{
  "C_Cpp.clang_format_style": "file",
  "editor.formatOnSave": true,
  "[cpp]": {
    "editor.defaultFormatter": "xaver.clang-format"
  },
  "clangd.arguments": [
    "--clang-tidy",
    "--header-insertion=never"
  ]
}
```

### CLion / IntelliJ

1. Go to **Settings → Editor → Code Style → C/C++**
2. Click **Set from...** → **Predefined Style** → **Google**
3. Enable **Enable ClangFormat** with **clang-format binary**
4. Check **Format on save**

### Vim/Neovim

Add to your config:

```vim
" Auto-format on save
autocmd BufWritePre *.cpp,*.h :silent! !clang-format -i %
```

## Key Style Rules

### Naming Conventions

- **Files**: `snake_case.cpp`, `snake_case.h`
- **Classes/Structs**: `CamelCase`
- **Functions**: `CamelCase()`
- **Variables**: `snake_case`
- **Constants**: `kCamelCase`
- **Member variables**: `snake_case_` (trailing underscore)
- **Macros**: `UPPER_CASE`

### Formatting

- **Indentation**: 2 spaces (no tabs)
- **Line length**: 80 characters (soft limit)
- **Braces**: Attached style (K&R)
- **Pointers**: `Type* ptr` (left-aligned)

### Header Guards

Use the pattern: `PATH_TO_FILE_H_`

Example: `include/data_node/address_record.h` → `DATA_NODE_ADDRESS_RECORD_H_`

### Include Order

1. Related header (for .cpp files)
2. C system headers
3. C++ standard library headers
4. Other libraries' headers
5. Project headers

Each group should be alphabetically sorted.

Example:

```cpp
#include "data_node/my_class.h"  // Related header

#include <sys/types.h>           // C system headers
#include <unistd.h>

#include <string>                // C++ standard library
#include <vector>

#include <gtest/gtest.h>         // Other libraries

#include "data_node/other.h"     // Project headers
```

## Pre-commit Hooks

If you installed pre-commit, it will automatically:
- Format your code with clang-format before each commit
- Run clang-tidy checks
- Prevent commits with style violations

To bypass (not recommended):

```bash
git commit --no-verify
```

## CI/CD Integration

The style checks are integrated into the build process. To ensure your code passes CI:

```bash
# Format code
cmake --build build --target format

# Check formatting
cmake --build build --target check-format

# Build and test
cmake --build build
./build/tests
```

## Common Issues

### Issue: "clang-format not found"

**Solution**: Install clang-format using your package manager (see Installation section)

### Issue: "Style violations in CI but not locally"

**Solution**: Ensure you're using the same clang-format version. Check with:

```bash
clang-format --version
```

### Issue: "Too many style violations to fix manually"

**Solution**: Run the automatic formatter:

```bash
./scripts/format_code.sh
```

## Resources

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)
