cmake_minimum_required(VERSION 3.15)
project(geocoding_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies (installed via vcpkg)
find_package(Crow CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(rapidcheck CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Data Node executable
add_executable(data_node_server
    data_node_main.cpp
    src/data_node/csv_parser.cpp
    src/data_node/address_normalizer.cpp
    src/data_node/radix_tree_index.cpp
    src/data_node/forward_index.cpp
    src/data_node/data_node.cpp
)
target_link_libraries(data_node_server PRIVATE Crow::Crow)

# Gateway Node executable (placeholder for future implementation)
add_executable(gateway_server
    gateway_main.cpp
    src/gateway/gateway_server.cpp
)
target_link_libraries(gateway_server PRIVATE Crow::Crow)

# Test executable
enable_testing()
add_executable(tests
    test/data_node/csv_parser_test.cpp
    test/data_node/address_normalizer_test.cpp
    test/data_node/radix_tree_index_test.cpp
    test/data_node/forward_index_test.cpp
    test/data_node/data_node_test.cpp
    test/data_node/property_tests.cpp
    src/data_node/csv_parser.cpp
    src/data_node/address_normalizer.cpp
    src/data_node/radix_tree_index.cpp
    src/data_node/forward_index.cpp
    src/data_node/data_node.cpp
)
target_link_libraries(tests PRIVATE 
    GTest::gtest 
    GTest::gtest_main
    rapidcheck
)

include(GoogleTest)
gtest_discover_tests(tests)
