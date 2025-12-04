# Geocoding System - Data Nodes

A distributed geocoding system with data nodes that provide efficient address search capabilities using RadixTree and ForwardIndex structures.

## Code Style

This project follows the **Google C++ Style Guide**. See [docs/STYLE_GUIDE.md](docs/STYLE_GUIDE.md) for detailed information.

## Option 1: Run with Docker Compose (Recommended)

The easiest way to run the complete system is using Docker Compose, which starts both data nodes and the gateway automatically.

> **⏱️ Note**: The first build takes 5-10 minutes as it compiles C++ dependencies (gRPC, Protobuf, etc.). Subsequent builds are much faster (~30-60 seconds) thanks to Docker layer caching. See [DOCKER_BUILD_GUIDE.md](DOCKER_BUILD_GUIDE.md) for optimization tips.

### Start All Services

```bash
# Build and start all services (2 data nodes + gateway)
docker-compose up --build

# Or run in detached mode
docker-compose up -d --build
```

### Stop All Services

```bash
docker-compose down
```

### View Logs

```bash
# View all logs
docker-compose logs

# View logs for specific service
docker-compose logs gateway
docker-compose logs data_node_0
docker-compose logs data_node_1

# Follow logs in real-time
docker-compose logs -f
```

### Test the Gateway API

Once the services are running, you can test the gateway:

#### Option 1: Use the Web Interface (Recommended)

Open the beautiful test portal in your browser:

```bash
# macOS
open web/index.html

# Linux
xdg-open web/index.html

# Or serve with Python
cd web && python3 -m http.server 8000
# Then open: http://localhost:8000
```

The web interface provides:
- 🎨 Beautiful, modern UI
- 🔍 Real-time search with example queries
- 📊 Result ranking and relevance scores
- 🗺️ Google Maps integration
- 📱 Mobile responsive design

#### Option 2: Use curl

```bash
# Health check
curl http://localhost:18080/health

# Search for an address
curl -X POST http://localhost:18080/api/findAddress \
  -H "Content-Type: application/json" \
  -d '{"address": "Salinas"}'
```

### Using Custom Data Files

To use custom data files, modify the volume mounts in `docker-compose.yml` or place your CSV files in the `./data` directory with the names:
- `shard_0_data_demo.csv`
- `shard_1_data_demo.csv`

## Option 2: Run with Docker (Manual)

You can also run individual containers manually if you need more control.

### Build the Docker Image

```bash
docker build -t geocoding-system .
```

### Run Data Node (Shard 0)

```bash
docker run -p 50051:50051 \
  -e SERVICE_TYPE=data_node \
  -e SHARD_ID=0 \
  geocoding-system
```

### Run Data Node (Shard 1)

```bash
docker run -p 50052:50052 \
  -e SERVICE_TYPE=data_node \
  -e SHARD_ID=1 \
  geocoding-system
```

### Run Gateway Server

```bash
docker run -p 18080:18080 \
  -e SERVICE_TYPE=gateway \
  -e DATA_NODE_0=host.docker.internal:50051 \
  -e DATA_NODE_1=host.docker.internal:50052 \
  geocoding-system
```

### Docker Environment Variables

**For Data Nodes:**
- `SERVICE_TYPE`: Set to `data_node` (required)
- `SHARD_ID`: Shard identifier (default: 0)
- `DATA_FILE_PATH`: Path to CSV data file (default: `data/shard_{SHARD_ID}_data_demo.csv`)
- `GRPC_PORT`: gRPC port to listen on (default: 50051 + SHARD_ID)

**For Gateway:**
- `SERVICE_TYPE`: Set to `gateway` (required)
- `HTTP_PORT`: HTTP port for REST API (default: 18080)
- `DATA_NODE_0`: Address of first data node (default: localhost:50051)
- `DATA_NODE_1`: Address of second data node (default: localhost:50052)

### Using Custom Data Files

To use custom data files, mount them as volumes:

```bash
docker run -p 50051:50051 \
  -v /path/to/your/data:/app/data \
  -e SERVICE_TYPE=data_node \
  -e SHARD_ID=0 \
  -e DATA_FILE_PATH=/app/data/custom_shard_0.csv \
  geocoding-system
```

## Option 3: Build and Run Locally (without Docker)

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


## Development

### Code Formatting

To ensure code follows Google C++ Style Guide:

```bash
# Install clang-format (macOS)
brew install clang-format

# Install clang-format (Linux)
sudo apt-get install clang-format

# Format all code
cmake --build build --target format

# Or use the script
./scripts/format_code.sh

# Check formatting without modifying files
cmake --build build --target check-format
```

### Running Tests

```bash
# Build and run all tests
cmake --build build
./build/tests

# Run specific test suite
./build/tests --gtest_filter="AddressRecordTest.*"
```

For more details on code style enforcement, see [docs/STYLE_GUIDE.md](docs/STYLE_GUIDE.md).
