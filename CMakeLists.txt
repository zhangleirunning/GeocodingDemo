cmake_minimum_required(VERSION 3.15)
project(crow_hello_world)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Crow (installed via vcpkg)
find_package(Crow CONFIG REQUIRED)

add_executable(hello_world main.cpp)
target_link_libraries(hello_world PRIVATE Crow::Crow)
