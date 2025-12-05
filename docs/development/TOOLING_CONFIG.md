# Tooling Configuration Files

## Overview

The project root contains several configuration files for development tools. These files **must** remain in the root directory as they are required by various tools.

## Configuration Files

### `.clang-format`

**Purpose:** Code formatting rules for C++ files

**Used by:**
- clang-format command line tool
- IDE integrations (VS Code, CLion, etc.)
- CI/CD formatting checks

**How it works:**
- Defines formatting rules (indentation, braces, spacing, etc.)
- Based on Google C++ Style Guide
- Applied automatically by `cmake --build build --target format`

**Location requirement:** Must be in project root. clang-format searches for this file by walking up from source files.

**Documentation:** See [STYLE_GUIDE.md](STYLE_GUIDE.md)

---

### `.clang-tidy`

**Purpose:** Static analysis and linting rules for C++ code

**Used by:**
- clang-tidy command line tool
- CMake integration (CMAKE_CXX_CLANG_TIDY)
- IDE integrations

**How it works:**
- Performs static analysis on C++ code
- Checks for bugs, performance issues, style violations
- Integrated into CMake build if clang-tidy is installed

**Location requirement:** Must be in project root or build directory.

**Configuration:**
```yaml
Checks: '-*,readability-*,modernize-*,performance-*'
```

---

### `.pre-commit-config.yaml`

**Purpose:** Git pre-commit hooks configuration

**Used by:**
- pre-commit framework
- Git hooks

**How it works:**
- Runs checks before each commit
- Can run formatters, linters, tests
- Prevents committing code that doesn't meet standards

**Location requirement:** Must be in repository root with this exact filename.

**Setup:**
```bash
# Install pre-commit
pip install pre-commit

# Install hooks
pre-commit install

# Run manually
pre-commit run --all-files
```

**Configuration:**
```yaml
repos:
  - repo: https://github.com/pre-commit/mirrors-clang-format
    hooks:
      - id: clang-format
```

---

## Why These Files Are in Root

### Technical Requirements

1. **Tool Discovery**
   - Tools search for config files starting from source file location
   - Walk up directory tree until found
   - Must be in a parent directory of all source files

2. **Standard Conventions**
   - Industry standard to place these in root
   - Expected by IDEs and CI/CD systems
   - Matches behavior of other projects (LLVM, Chromium, etc.)

3. **Git Integration**
   - `.pre-commit-config.yaml` must be in repository root
   - Git hooks expect configuration at repository level

### Cannot Be Moved

❌ **These files cannot be moved to a subdirectory** because:
- Tools won't find them
- Would break IDE integrations
- Would break CI/CD pipelines
- Would violate tool conventions

## Other Root Configuration Files

These are also standard and necessary in root:

### `.gitignore`
- **Purpose:** Specifies files Git should ignore
- **Required location:** Repository root
- **Cannot move:** Git requires it in root

### `.dockerignore`
- **Purpose:** Specifies files Docker should ignore when building
- **Required location:** Same directory as Dockerfile (or root)
- **Cannot move:** Docker requires it in build context root

### `.vscode/`
- **Purpose:** VS Code workspace settings
- **Required location:** Workspace root
- **Cannot move:** VS Code expects it in workspace root

## Best Practices

### 1. Keep Them Updated

```bash
# Update clang-format rules
vim .clang-format

# Test formatting
cmake --build build --target format
```

### 2. Document Changes

When modifying these files:
- Document why in commit message
- Update this guide if behavior changes
- Test with the actual tools

### 3. Consistent Across Team

- Commit these files to version control
- Ensure all developers use same versions
- Include in onboarding documentation

## Viewing Configuration

### Check Current Settings

```bash
# View clang-format config
cat .clang-format

# View clang-tidy config
cat .clang-tidy

# View pre-commit config
cat .pre-commit-config.yaml
```

### Test Configuration

```bash
# Test clang-format
clang-format --style=file --dry-run src/data_node/*.cpp

# Test clang-tidy
clang-tidy src/data_node/csv_parser.cpp

# Test pre-commit
pre-commit run --all-files
```

## Related Documentation

- [Style Guide](STYLE_GUIDE.md) - Code style rules and enforcement
- [Quick Style Reference](QUICK_STYLE_REFERENCE.md) - Quick lookup guide
- [Style Setup Summary](STYLE_SETUP_SUMMARY.md) - Setting up formatting tools

## Common Questions

### Q: Can I move these to a `.config/` directory?

**A:** No. Tools require these files in specific locations (usually root). Moving them will break tool discovery.

### Q: Why are they hidden (start with `.`)?

**A:** Unix convention for configuration files. They don't clutter directory listings but are still accessible.

### Q: Do I need all three?

**A:**
- `.clang-format` - **Required** for consistent formatting
- `.clang-tidy` - **Recommended** for code quality
- `.pre-commit-config.yaml` - **Optional** but recommended for automation

### Q: Can I have different configs for different directories?

**A:** Yes! You can place additional `.clang-format` files in subdirectories. The tool uses the closest one found when walking up the tree.

## Summary

✅ **Keep these files in root** - Required by tools
✅ **Commit to version control** - Ensure consistency
✅ **Document changes** - Help team understand
✅ **Test before committing** - Ensure they work

These files are small, necessary, and follow industry standards. They should remain in the project root.
