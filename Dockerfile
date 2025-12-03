FROM debian:bookworm-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the project (builds both data_node_server and gateway_server)
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    make data_node_server gateway_server

# Runtime stage - smaller image
FROM debian:bookworm-slim

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy built executables from builder stage
COPY --from=builder /app/build/data_node_server /app/data_node_server
COPY --from=builder /app/build/gateway_server /app/gateway_server

# Copy data files
COPY --from=builder /app/data /app/data

# Create directory for custom data files
RUN mkdir -p /app/data

# Environment variables with defaults
ENV SHARD_ID=0
ENV DATA_FILE_PATH=""
ENV GRPC_PORT=""
ENV SERVICE_TYPE=data_node

# Expose ports for data nodes (50051, 50052) and gateway (18080)
EXPOSE 50051 50052 18080

# Entrypoint script to run the appropriate service
COPY docker-entrypoint.sh /app/docker-entrypoint.sh
RUN chmod +x /app/docker-entrypoint.sh

ENTRYPOINT ["/app/docker-entrypoint.sh"]
