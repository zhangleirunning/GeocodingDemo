# Quick Google C++ Style Reference

## Installation (One-time setup)

```bash
# macOS
brew install clang-format llvm

# Linux (Ubuntu/Debian)
sudo apt-get install clang-format clang-tidy
```

## Daily Usage

### Before Committing Code

```bash
# Format your code
./scripts/format_code.sh

# Or using CMake
cmake --build build --target format
```

### Check Your Code

```bash
# Check if code is properly formatted
./scripts/check_style.sh

# Or using CMake
cmake --build build --target check-format
```

## Quick Style Rules

### ✅ DO

```cpp
// Header guards
#ifndef DATA_NODE_MY_CLASS_H_
#define DATA_NODE_MY_CLASS_H_

// Includes (sorted alphabetically within groups)
#include <cmath>
#include <string>

#include "data_node/other.h"

// Constants with k prefix
const double kEpsilon = 1e-9;
const int kMaxRetries = 3;

// Classes/Structs: CamelCase
class MyClass {
 public:
  // Functions: CamelCase
  void DoSomething();

  // Getters/Setters
  int value() const { return value_; }
  void set_value(int value) { value_ = value; }

 private:
  // Member variables: snake_case with trailing underscore
  int value_;
  std::string name_;
};

// Variables: snake_case
int my_variable = 42;
std::string user_name = "Alice";

// 2-space indentation
if (condition) {
  DoSomething();
}

// Pointer/reference alignment: left
int* ptr = nullptr;
const std::string& name = GetName();
```

### ❌ DON'T

```cpp
// Wrong header guard
#ifndef MY_CLASS_H  // Missing path and trailing underscore

// Wrong constant naming
const double epsilon = 1e-9;  // Should be kEpsilon
const int MAX_RETRIES = 3;    // Should be kMaxRetries

// Wrong class naming
class my_class {  // Should be MyClass
class myClass {   // Should be MyClass

// Wrong function naming
void do_something();  // Should be DoSomething()
void dosomething();   // Should be DoSomething()

// Wrong variable naming
int MyVariable = 42;        // Should be my_variable
std::string UserName = "";  // Should be user_name

// Wrong member variable naming
class MyClass {
  int value;      // Should be value_
  int m_value;    // Should be value_
  int _value;     // Should be value_
};

// 4-space indentation (wrong)
if (condition) {
    DoSomething();
}

// Pointer/reference alignment: right (wrong)
int *ptr = nullptr;         // Should be int* ptr
const std::string &name;    // Should be const std::string& name
```

## Common Patterns

### Struct with Constructor and Operators

```cpp
struct AddressRecord {
  std::string id;
  double longitude;
  double latitude;

  // Default constructor
  AddressRecord() : longitude(0.0), latitude(0.0) {}

  // Parameterized constructor
  AddressRecord(const std::string& id_, double lon, double lat)
      : id(id_), longitude(lon), latitude(lat) {}

  // Equality operator
  bool operator==(const AddressRecord& other) const {
    const double kEpsilon = 1e-9;
    return id == other.id &&
           std::abs(longitude - other.longitude) < kEpsilon &&
           std::abs(latitude - other.latitude) < kEpsilon;
  }
};
```

### Class with Private Members

```cpp
class DataNode {
 public:
  DataNode(int shard_id, const std::string& data_path);

  bool Initialize();
  std::vector<AddressRecord> Search(const std::vector<std::string>& terms);

  int shard_id() const { return shard_id_; }

 private:
  int shard_id_;
  std::string data_path_;
  std::unique_ptr<RadixTreeIndex> index_;

  void BuildIndexes();
};
```

### Include Order

```cpp
// 1. Related header (for .cpp files)
#include "data_node/my_class.h"

// 2. C system headers
#include <sys/types.h>
#include <unistd.h>

// 3. C++ standard library (alphabetically)
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

// 4. Other libraries
#include <gtest/gtest.h>

// 5. Project headers (alphabetically)
#include "data_node/address_record.h"
#include "data_node/csv_parser.h"
```

## IDE Setup

### VS Code

The project includes `.vscode/settings.json` with proper configuration.
Just install the **C/C++** extension and formatting will work automatically on save.

### CLion

1. Settings → Editor → Code Style → C/C++
2. Set from... → Predefined Style → Google
3. Enable "Reformat code" on save

## Troubleshooting

### "My code looks different after formatting"

This is expected! The formatter ensures consistency. Review the changes and commit them.

### "I want to disable formatting for a section"

```cpp
// clang-format off
int matrix[3][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};
// clang-format on
```

Use sparingly! Only for cases where manual formatting improves readability.

### "Formatting breaks my code"

This shouldn't happen. If it does:
1. Check for syntax errors first
2. Report the issue - it might be a clang-format bug

## Resources

- Full guide: [STYLE_GUIDE.md](STYLE_GUIDE.md)
- Google C++ Style Guide: https://google.github.io/styleguide/cppguide.html
- clang-format docs: https://clang.llvm.org/docs/ClangFormat.html
