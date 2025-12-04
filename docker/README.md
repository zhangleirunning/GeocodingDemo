# Docker Configuration

This directory contains all Docker-related files for the geocoding system.

## Files

- **Dockerfile** - Multi-stage build for data nodes and gateway
- **docker-compose.yml** - Base configuration for all services
- **docker-compose.dev.yml** - Development overrides
- **docker-compose.prod.yml** - Production overrides
- **entrypoint.sh** - Container entrypoint script

## Quick Start

### Development

```bash
# From project root
docker-compose up --build

# Or explicitly use docker directory
docker-compose -f docker/docker-compose.yml up --build
```

### Production

```bash
# Use production overrides
docker-compose -f docker/docker-compose.yml -f docker/docker-compose.prod.yml up -d
```

### Development with Hot Reload

```bash
# Use development overrides
docker-compose -f docker/docker-compose.yml -f docker/docker-compose.dev.yml up
```

## Services

### Data Node 0 (Shard 0)
- **Port:** 50051
- **Data:** `../data/shard_0_data_demo.csv`
- **Purpose:** First data shard

### Data Node 1 (Shard 1)
- **Port:** 50052
- **Data:** `../data/shard_1_data_demo.csv`
- **Purpose:** Second data shard

### Gateway
- **Port:** 18080
- **Purpose:** HTTP API and web frontend
- **Endpoints:**
  - `http://localhost:18080/` - Web UI
  - `http://localhost:18080/health` - Health check
  - `http://localhost:18080/api/findAddress` - Search API

## Environment Variables

### Data Nodes
- `SERVICE_TYPE=data_node` - Service type
- `SHARD_ID` - Shard identifier (0 or 1)
- `DATA_FILE_PATH` - Path to CSV data file
- `GRPC_PORT` - gRPC port (50051 or 50052)
- `LOG_LEVEL` - Logging level (DEBUG, INFO, WARN, ERROR)

### Gateway
- `SERVICE_TYPE=gateway` - Service type
- `HTTP_PORT` - HTTP server port (default: 18080)
- `DATA_NODE_0` - Address of first data node
- `DATA_NODE_1` - Address of second data node
- `GRPC_TIMEOUT_MS` - gRPC timeout in milliseconds
- `LOG_LEVEL` - Logging level

## Build Process

The Dockerfile uses a multi-stage build:

1. **Builder Stage**
   - Installs vcpkg and dependencies
   - Compiles C++ code
   - Generates gRPC/Protobuf files

2. **Runtime Stage**
   - Minimal Debian image
   - Only runtime dependencies
   - Copies compiled binaries
   - ~200MB smaller than builder

## Volumes

- `../data:/app/data:ro` - Read-only data files
- `../web:/app/web` - Web frontend files (dev only)

## Networks

- `geocoding_network` - Bridge network for inter-service communication

## Health Checks

All services include health checks:
- **Data Nodes:** Placeholder check (can be enhanced)
- **Gateway:** HTTP health endpoint check

## Commands

```bash
# Build images
docker-compose build

# Start services
docker-compose up -d

# View logs
docker-compose logs -f

# Stop services
docker-compose down

# Rebuild and restart
docker-compose up -d --build

# Scale data nodes (if needed)
docker-compose up -d --scale data_node_0=2
```

## Troubleshooting

### Build fails
```bash
# Clean build
docker-compose down
docker system prune -f
docker-compose build --no-cache
```

### Services won't start
```bash
# Check logs
docker-compose logs gateway
docker-compose logs data_node_0

# Check status
docker-compose ps
```

### Port conflicts
```bash
# Check what's using ports
lsof -i :18080
lsof -i :50051
lsof -i :50052
```

## Development Tips

1. **Use dev compose** for faster iteration
2. **Mount volumes** to avoid rebuilds
3. **Use builder stage** for debugging
4. **Check logs** frequently

## Production Tips

1. **Use prod compose** for resource limits
2. **Enable logging** with rotation
3. **Set restart policies** to always
4. **Monitor resource usage**
5. **Use health checks** for orchestration

## Architecture

```
┌─────────────────────────────────────┐
│   Browser: http://localhost:18080   │
└─────────────────┬───────────────────┘
                  │
          ┌───────▼────────┐
          │    Gateway     │
          │   Port 18080   │
          └───────┬────────┘
                  │ gRPC
         ┌────────┴────────┐
         │                 │
    ┌────▼─────┐     ┌────▼─────┐
    │ Data     │     │ Data     │
    │ Node 0   │     │ Node 1   │
    │ :50051   │     │ :50052   │
    └──────────┘     └──────────┘
```

## Next Steps

- Add monitoring (Prometheus/Grafana)
- Add tracing (Jaeger/Zipkin)
- Add load balancing
- Add SSL/TLS certificates
- Add backup strategies
