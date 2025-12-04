# Project Restructuring Plan

## Current Issues

1. **Root directory clutter** - Too many loose files (`*.cpp`, `main.cpp`, test files)
2. **Mixed concerns** - Test utilities mixed with main executables
3. **Documentation scattered** - Some docs in root, some in `docs/`
4. **Unclear entry points** - Multiple `main.cpp` files
5. **Build artifacts** - `build/` directory should be gitignored but visible

## Recommended Structure

```
geocoding-system/
├── .github/                    # CI/CD workflows (future)
│   └── workflows/
│       ├── build.yml
│       └── test.yml
│
├── .kiro/                      # Kiro specs (keep as is)
│   └── specs/
│
├── apps/                       # 🆕 Application entry points
│   ├── data_node/
│   │   └── main.cpp           # data_node_main.cpp → here
│   ├── gateway/
│   │   └── main.cpp           # gateway_main.cpp → here
│   └── tools/                 # 🆕 Utility applications
│       ├── grpc_client.cpp    # test_grpc_client.cpp → here
│       └── search_tool.cpp    # test_search.cpp → here
│
├── src/                        # Core library code (keep structure)
│   ├── data_node/
│   │   ├── address_normalizer.cpp
│   │   ├── csv_parser.cpp
│   │   ├── data_node.cpp
│   │   ├── forward_index.cpp
│   │   └── radix_tree_index.cpp
│   └── gateway/
│       └── gateway_server.cpp
│
├── include/                    # Public headers (keep structure)
│   ├── data_node/
│   │   ├── address_normalizer.h
│   │   ├── address_record.h
│   │   ├── csv_parser.h
│   │   ├── data_node.h
│   │   ├── forward_index.h
│   │   └── radix_tree_index.h
│   └── gateway/
│       └── gateway_server.h
│
├── test/                       # Tests (keep structure)
│   ├── data_node/
│   │   ├── *_test.cpp
│   │   └── property_tests.cpp
│   ├── gateway/
│   │   ├── gateway_server_test.cpp
│   │   └── gateway_integration_test.cpp
│   └── fixtures/              # 🆕 Rename test_data → fixtures
│       ├── valid_addresses.csv
│       ├── malformed_addresses.csv
│       └── empty_optional_fields.csv
│
├── proto/                      # Protocol buffers (keep as is)
│   └── data_node.proto
│
├── data/                       # Sample data (keep as is)
│   ├── shard_0_data_demo.csv
│   └── shard_1_data_demo.csv
│
├── web/                        # Web frontend UI
│   ├── index.html
│   └── README.md
│
├── scripts/                    # Build & utility scripts (keep)
│   ├── format_code.sh
│   ├── check_style.sh
│   ├── offline_data_processor.py
│   └── README.md
│
├── docs/                       # 🔄 Reorganize documentation
│   ├── architecture/          # 🆕 Architecture docs
│   │   ├── OVERVIEW.md
│   │   ├── DATA_NODE.md
│   │   └── GATEWAY.md
│   ├── development/           # 🆕 Development guides
│   │   ├── STYLE_GUIDE.md
│   │   ├── TESTING_GUIDE.md
│   │   └── CONTRIBUTING.md
│   ├── deployment/            # 🆕 Deployment guides
│   │   ├── DOCKER_GUIDE.md
│   │   └── PRODUCTION.md
│   └── api/                   # 🆕 API documentation
│       └── REST_API.md
│
├── docker/                     # 🆕 Docker-related files
│   ├── Dockerfile
│   ├── docker-compose.yml
│   ├── docker-compose.dev.yml # 🆕 Development compose
│   ├── docker-compose.prod.yml # 🆕 Production compose
│   └── entrypoint.sh
│
├── config/                     # 🆕 Configuration files
│   ├── data_node.example.json
│   └── gateway.example.json
│
├── build/                      # Build artifacts (gitignored)
├── .vscode/                    # Editor config (keep)
├── .clang-format              # Code style (keep)
├── .clang-tidy                # Linting (keep)
├── .dockerignore              # Docker ignore (keep)
├── .gitignore                 # Git ignore (keep)
├── .pre-commit-config.yaml    # Pre-commit hooks (keep)
├── CMakeLists.txt             # Build config (update paths)
├── vcpkg.json                 # Dependencies (keep)
├── README.md                  # Main readme (keep)
├── CHANGELOG.md               # 🆕 Version history
└── LICENSE                    # 🆕 License file
```

## Migration Steps

### Phase 1: Reorganize Applications (Priority: High)

```bash
# 1. Create apps directory structure
mkdir -p apps/data_node apps/gateway apps/tools

# 2. Move main files
mv data_node_main.cpp apps/data_node/main.cpp
mv gateway_main.cpp apps/gateway/main.cpp
mv test_grpc_client.cpp apps/tools/grpc_client.cpp
mv test_search.cpp apps/tools/search_tool.cpp

# 3. Remove old main.cpp if unused
rm main.cpp  # or move to apps/tools/ if needed

# 4. Update CMakeLists.txt paths
```

### Phase 2: Reorganize Documentation (Priority: Medium)

```bash
# 1. Create docs structure
mkdir -p docs/architecture docs/development docs/deployment docs/api

# 2. Move and rename existing docs
mv docs/STYLE_GUIDE.md docs/development/
mv docs/TESTING_GUIDE.md docs/development/
mv docs/DOCKER_BUILD_GUIDE.md docs/deployment/DOCKER_GUIDE.md

# 3. Move root-level docs
mv DOCKER_BUILD_GUIDE.md docs/deployment/ 2>/dev/null || true
mv TESTING_GUIDE.md docs/development/ 2>/dev/null || true
mv WEB_FRONTEND_INTEGRATION.md docs/deployment/ 2>/dev/null || true
mv QUICK_START.md docs/deployment/ 2>/dev/null || true

# 4. Create new architecture docs
```

### Phase 3: Reorganize Docker Files (Priority: Medium)

```bash
# 1. Create docker directory
mkdir -p docker

# 2. Move Docker files
mv Dockerfile docker/
mv docker-compose.yml docker/
mv docker-entrypoint.sh docker/entrypoint.sh

# 3. Update docker-compose.yml to reference docker/Dockerfile
```

### Phase 4: Reorganize Test Data (Priority: Low)

```bash
# Rename test_data to fixtures
mv test/test_data test/fixtures
```

### Phase 5: Add Missing Files (Priority: Low)

```bash
# 1. Create CHANGELOG.md
touch CHANGELOG.md

# 2. Create LICENSE
touch LICENSE

# 3. Create config examples
mkdir -p config
touch config/data_node.example.json
touch config/gateway.example.json

# 4. Create architecture docs
touch docs/architecture/OVERVIEW.md
touch docs/architecture/DATA_NODE.md
touch docs/architecture/GATEWAY.md

# 5. Create API docs
touch docs/api/REST_API.md
```

## Updated CMakeLists.txt Structure

```cmake
cmake_minimum_required(VERSION 3.15)
project(geocoding_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Crow CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(rapidcheck CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Generate gRPC/Protobuf files
# ... (keep existing proto generation)

# Core library (optional - for better organization)
add_library(geocoding_core STATIC
    src/data_node/csv_parser.cpp
    src/data_node/address_normalizer.cpp
    src/data_node/radix_tree_index.cpp
    src/data_node/forward_index.cpp
    src/data_node/data_node.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(geocoding_core PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
)

# Gateway library
add_library(geocoding_gateway STATIC
    src/gateway/gateway_server.cpp
    src/data_node/address_normalizer.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(geocoding_gateway PUBLIC
    Crow::Crow
    gRPC::grpc++
    protobuf::libprotobuf
)

# Data Node executable
add_executable(data_node_server
    apps/data_node/main.cpp
)
target_link_libraries(data_node_server PRIVATE
    geocoding_core
    Crow::Crow
)

# Gateway executable
add_executable(gateway_server
    apps/gateway/main.cpp
)
target_link_libraries(gateway_server PRIVATE
    geocoding_gateway
)

# Tools
add_executable(grpc_client
    apps/tools/grpc_client.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(grpc_client PRIVATE
    gRPC::grpc++
    protobuf::libprotobuf
)

# Tests
enable_testing()
add_executable(tests
    test/data_node/address_record_test.cpp
    test/data_node/csv_parser_test.cpp
    test/data_node/address_normalizer_test.cpp
    test/data_node/radix_tree_index_test.cpp
    test/data_node/forward_index_test.cpp
    test/data_node/data_node_test.cpp
    test/data_node/property_tests.cpp
    test/gateway/gateway_server_test.cpp
    test/gateway/gateway_integration_test.cpp
)
target_link_libraries(tests PRIVATE
    geocoding_core
    geocoding_gateway
    GTest::gtest
    GTest::gtest_main
    rapidcheck
)

include(GoogleTest)
gtest_discover_tests(tests)
```

## Benefits of New Structure

### 1. **Clearer Organization**
- ✅ Entry points in `apps/`
- ✅ Libraries in `src/`
- ✅ Tests in `test/`
- ✅ Docs organized by category

### 2. **Better Scalability**
- ✅ Easy to add new applications
- ✅ Easy to add new services
- ✅ Clear separation of concerns

### 3. **Improved Developer Experience**
- ✅ Easier to find files
- ✅ Clearer project structure
- ✅ Better documentation organization

### 4. **Production Ready**
- ✅ Separate dev/prod Docker configs
- ✅ Configuration examples
- ✅ Proper documentation structure

## Priority Recommendations

### Do Now (High Priority):
1. ✅ Move `*_main.cpp` to `apps/` directory
2. ✅ Update CMakeLists.txt paths
3. ✅ Move Docker files to `docker/` directory
4. ✅ Update .gitignore to exclude `build/`

### Do Soon (Medium Priority):
5. ✅ Reorganize documentation into categories
6. ✅ Create architecture documentation
7. ✅ Add CHANGELOG.md
8. ✅ Create configuration examples

### Do Later (Low Priority):
9. ✅ Rename `test_data` to `fixtures`
10. ✅ Add LICENSE file
11. ✅ Create API documentation
12. ✅ Add GitHub Actions workflows

## Migration Script

I can create an automated migration script if you want to proceed with the restructuring. Would you like me to:

1. Create a migration script (`scripts/migrate_structure.sh`)?
2. Update CMakeLists.txt with new paths?
3. Update Dockerfile with new paths?
4. Create the new documentation structure?

Let me know which parts you'd like to implement first!
