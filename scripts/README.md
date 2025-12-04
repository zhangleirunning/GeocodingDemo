# Scripts Directory

Utility scripts for the Geocoding project.

## Code Style Scripts

### `format_code.sh`
Automatically formats all C++ code according to the Google C++ Style Guide.

**Requirements:**
- `clang-format` (install: `brew install llvm` on macOS)

**Usage:**
```bash
./scripts/format_code.sh
```

**What it does:**
- Finds all `.cpp` and `.h` files in `include/`, `src/`, and `test/`
- Formats them according to `.clang-format` configuration
- Modifies files in-place

### `check_style.sh`
Checks C++ code style compliance without modifying files.

**Requirements:**
- `clang-format` (required)
- `clang-tidy` (optional, for additional checks)

**Usage:**
```bash
./scripts/check_style.sh
```

**What it does:**
- Checks if any files need reformatting
- Runs clang-tidy static analysis (if available)
- Exits with error if style violations found

**Installation:**
```bash
# macOS
brew install llvm

# Add to PATH (add to ~/.zshrc or ~/.bash_profile)
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Linux (Debian/Ubuntu)
sudo apt-get install clang-format clang-tidy
```

## Data Processing Scripts

### `offline_data_processor.py`
Python script for processing and sharding address data.

**Requirements:**
- Python 3.x

**Usage:**
```bash
python3 scripts/offline_data_processor.py <input_file> <output_prefix> <num_shards>
```

**Example:**
```bash
python3 scripts/offline_data_processor.py data/addresses.csv data/shard 2
# Creates: data/shard_0.csv, data/shard_1.csv
```

## Verification Scripts

### `verify_web_rename.sh`
Verifies that the frontend → web directory rename was completed successfully.

**Usage:**
```bash
./scripts/verify_web_rename.sh
```

**What it checks:**
- Directory structure
- Code references
- Docker configuration
- Documentation updates
- Build artifacts

## Configuration Files

The scripts use these configuration files in the project root:

- **`.clang-format`** - Code formatting rules (Google C++ Style)
- **`.clang-tidy`** - Static analysis rules
- **`.pre-commit-config.yaml`** - Git pre-commit hooks (optional)

## CI/CD Integration

These scripts can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Check code style
  run: ./scripts/check_style.sh

- name: Format code
  run: ./scripts/format_code.sh
```

## IDE Integration

### VS Code
Add to `.vscode/settings.json`:
```json
{
  "C_Cpp.clang_format_path": "/opt/homebrew/opt/llvm/bin/clang-format",
  "C_Cpp.clang_format_style": "file",
  "editor.formatOnSave": true
}
```

### CLion
1. Go to Preferences → Editor → Code Style → C/C++
2. Set Scheme to "Project"
3. Enable "Enable ClangFormat"
4. Set ClangFormat binary path

## Troubleshooting

### "clang-format not found"
Install LLVM tools:
```bash
brew install llvm
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

### "Permission denied"
Make scripts executable:
```bash
chmod +x scripts/*.sh
```

### "Style violations found"
Run the formatter:
```bash
./scripts/format_code.sh
```

## Best Practices

1. **Before committing**: Run `./scripts/format_code.sh`
2. **In CI/CD**: Run `./scripts/check_style.sh`
3. **IDE setup**: Enable format-on-save
4. **Team workflow**: Ensure all developers have clang-format installed

## See Also

- [Style Guide](../docs/development/STYLE_GUIDE.md)
- [Quick Style Reference](../docs/development/QUICK_STYLE_REFERENCE.md)
- [Style Setup Summary](../docs/development/STYLE_SETUP_SUMMARY.md)
