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

# Copy only dependency files first (for better caching)
# This layer will be cached unless these files change
COPY vcpkg.json CMakeLists.txt ./
COPY proto ./proto

# Install dependencies via vcpkg (this is the slow part - ~5-7 minutes)
# This layer will be cached and reused on subsequent builds
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    echo "Dependencies installed and cached"

# Now copy the rest of the source code
# Changes to your code won't invalidate the dependency cache above
COPY include ./include
COPY src ./src
COPY data ./data
COPY *.cpp ./

# Build the project (only your code compiles here - ~30 seconds)
RUN cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    make data_node_server gateway_server -j$(nproc)

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
