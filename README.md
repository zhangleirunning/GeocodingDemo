# Crow C++ Hello World

A simple "Hello World" web server using the Crow C++ framework.

## Option 1: Run with Docker

The easiest way to run this project is using Docker:

```bash
# Build the Docker image
docker build -t crow-hello-world .

# Run the container
docker run -p 18080:18080 crow-hello-world
```

Visit http://localhost:18080/ in your browser to see "Hello world"

## Option 2: Run on Debian (without Docker)

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- vcpkg (C++ package manager)
- pkg-config

### Install Dependencies on Debian

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git curl zip unzip tar pkg-config
```

### Install vcpkg

If you haven't installed vcpkg yet:

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

Add vcpkg to environment variables (optional):
```bash
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

### Build the Project

Build the project using vcpkg toolchain:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
make
```

Or if you've set the VCPKG_ROOT environment variable:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
make
```

### Run the Server

```bash
./build/hello_world
```

The server will start on http://localhost:18080

Visit http://localhost:18080/ in your browser to see "Hello world"
