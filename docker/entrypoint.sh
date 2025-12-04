#!/bin/bash
set -e

# Docker entrypoint script for geocoding system
# Supports running either data_node_server or gateway_server

echo "========================================="
echo "Geocoding System Docker Container"
echo "========================================="
echo ""

# Determine which service to run
if [ "$SERVICE_TYPE" = "data_node" ]; then
    echo "Starting Data Node Server..."
    echo "Configuration:"
    echo "  SHARD_ID: ${SHARD_ID:-0}"
    echo "  DATA_FILE_PATH: ${DATA_FILE_PATH:-(default)}"
    echo "  GRPC_PORT: ${GRPC_PORT:-(default)}"
    echo ""

    exec /app/data_node_server

elif [ "$SERVICE_TYPE" = "gateway" ]; then
    echo "Starting Gateway Server..."
    echo "Configuration:"
    echo "  HTTP_PORT: ${HTTP_PORT:-18080}"
    echo "  DATA_NODE_0: ${DATA_NODE_0:-localhost:50051}"
    echo "  DATA_NODE_1: ${DATA_NODE_1:-localhost:50052}"
    echo ""

    exec /app/gateway_server

else
    echo "ERROR: Unknown SERVICE_TYPE: $SERVICE_TYPE"
    echo "Valid values: data_node, gateway"
    exit 1
fi
