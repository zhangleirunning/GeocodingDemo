cmake_minimum_required(VERSION 3.15)
project(geocoding_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies (installed via vcpkg)
find_package(Crow CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(rapidcheck CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Generate gRPC/Protobuf files
set(PROTO_FILES ${CMAKE_SOURCE_DIR}/proto/data_node.proto)
set(PROTO_OUTPUT_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# Get protobuf and grpc plugin paths
get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOBUF_PROTOC)
    get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION_DEBUG)
endif()
if(NOT PROTOBUF_PROTOC)
    get_target_property(PROTOBUF_PROTOC protobuf::protoc IMPORTED_LOCATION)
endif()

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

# Generate protobuf and gRPC sources
set(PROTO_SRCS ${PROTO_OUTPUT_DIR}/data_node.pb.cc)
set(PROTO_HDRS ${PROTO_OUTPUT_DIR}/data_node.pb.h)
set(GRPC_SRCS ${PROTO_OUTPUT_DIR}/data_node.grpc.pb.cc)
set(GRPC_HDRS ${PROTO_OUTPUT_DIR}/data_node.grpc.pb.h)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${PROTOBUF_PROTOC}
    ARGS --grpc_out=${PROTO_OUTPUT_DIR}
         --cpp_out=${PROTO_OUTPUT_DIR}
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
         -I${CMAKE_SOURCE_DIR}/proto
         ${PROTO_FILES}
    DEPENDS ${PROTO_FILES}
    COMMENT "Generating gRPC and Protobuf files"
)

# Include generated proto headers
include_directories(${PROTO_OUTPUT_DIR})

# Data Node executable
add_executable(data_node_server
    data_node_main.cpp
    src/data_node/csv_parser.cpp
    src/data_node/address_normalizer.cpp
    src/data_node/radix_tree_index.cpp
    src/data_node/forward_index.cpp
    src/data_node/data_node.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(data_node_server PRIVATE
    Crow::Crow
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

# Gateway Node executable
add_executable(gateway_server
    gateway_main.cpp
    src/gateway/gateway_server.cpp
    src/data_node/address_normalizer.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(gateway_server PRIVATE
    Crow::Crow
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

# Test gRPC client
add_executable(test_grpc_client
    test_grpc_client.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(test_grpc_client PRIVATE
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

# Quick search test
add_executable(test_search
    test_search.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(test_search PRIVATE
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

# Test executable
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
    src/data_node/csv_parser.cpp
    src/data_node/address_normalizer.cpp
    src/data_node/radix_tree_index.cpp
    src/data_node/forward_index.cpp
    src/data_node/data_node.cpp
    src/gateway/gateway_server.cpp
    ${PROTO_SRCS}
    ${GRPC_SRCS}
)
target_link_libraries(tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    rapidcheck
    Crow::Crow
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

include(GoogleTest)
gtest_discover_tests(tests)

# Code formatting and style checking targets
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/include/*.h
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/test/*.cpp
        ${CMAKE_SOURCE_DIR}/*.cpp
    )

    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format to format code"
    )

    add_custom_target(
        check-format
        COMMAND ${CLANG_FORMAT} -style=file --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking code formatting"
    )
endif()

find_program(CLANG_TIDY "clang-tidy")
if(CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY} -p=${CMAKE_BINARY_DIR})
endif()
