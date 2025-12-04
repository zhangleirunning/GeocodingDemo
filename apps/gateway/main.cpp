// Gateway Server Entry Point with HTTP API

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "gateway/gateway_server.h"

// Global pointer for signal handling
std::unique_ptr<GatewayServer> g_gateway_server;
volatile sig_atomic_t g_shutdown_requested = 0;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
  std::cout << "\n[INFO] Received signal " << signal
            << ", initiating graceful shutdown..." << std::endl;
  g_shutdown_requested = 1;

  if (g_gateway_server) {
    g_gateway_server->shutdown();
  }
}

// Get HTTP port from environment variable with default
int getHttpPort() {
  const char* env_port = std::getenv("HTTP_PORT");
  if (env_port) {
    try {
      int port = std::stoi(env_port);
      if (port <= 0 || port > 65535) {
        std::cerr << "[ERROR] HTTP_PORT must be between 1 and 65535"
                  << std::endl;
        return -1;
      }
      return port;
    } catch (const std::exception& e) {
      std::cerr << "[ERROR] Invalid HTTP_PORT: " << env_port << std::endl;
      return -1;
    }
  }

  // Default HTTP port
  return 18080;
}

// Get data node address from environment variable
std::string getDataNodeAddress(int node_index) {
  std::string env_var = "DATA_NODE_" + std::to_string(node_index);
  const char* env_address = std::getenv(env_var.c_str());
  if (env_address) {
    return std::string(env_address);
  }

  // Default addresses based on node index
  if (node_index == 0) {
    return "localhost:50051";
  } else if (node_index == 1) {
    return "localhost:50052";
  }

  return "";
}

// Get gRPC timeout from environment variable with default
int getGrpcTimeout() {
  const char* env_timeout = std::getenv("GRPC_TIMEOUT_MS");
  if (env_timeout) {
    try {
      int timeout = std::stoi(env_timeout);
      if (timeout <= 0) {
        std::cerr << "[WARNING] GRPC_TIMEOUT_MS must be positive, using default"
                  << std::endl;
        return 5000;
      }
      return timeout;
    } catch (const std::exception& e) {
      std::cerr << "[WARNING] Invalid GRPC_TIMEOUT_MS: " << env_timeout
                << ", using default" << std::endl;
    }
  }

  // Default timeout: 5 seconds
  return 5000;
}

int main(int argc, char* argv[]) {
  std::cout << "========================================" << std::endl;
  std::cout << "Gateway Server" << std::endl;
  std::cout << "========================================\n" << std::endl;

  // Get configuration from environment variables or use defaults
  int http_port = getHttpPort();
  if (http_port < 0) {
    return EXIT_FAILURE;
  }

  std::string data_node_0 = getDataNodeAddress(0);
  std::string data_node_1 = getDataNodeAddress(1);
  int grpc_timeout_ms = getGrpcTimeout();

  std::cout << "[INFO] Starting Gateway Server with configuration:" << std::endl;
  std::cout << "  HTTP port: " << http_port << std::endl;
  std::cout << "  Data Node 0: " << data_node_0 << std::endl;
  std::cout << "  Data Node 1: " << data_node_1 << std::endl;
  std::cout << "  gRPC timeout: " << grpc_timeout_ms << " ms\n" << std::endl;

  // Set up signal handlers for graceful shutdown
  std::signal(SIGINT, signalHandler);   // Ctrl+C
  std::signal(SIGTERM, signalHandler);  // Termination signal

  // Build gateway configuration
  GatewayConfig config;
  config.http_port = http_port;
  config.grpc_timeout_ms = grpc_timeout_ms;

  // Add data node configurations
  if (!data_node_0.empty()) {
    DataNodeConfig node0;
    node0.address = data_node_0;
    node0.shard_id = 0;
    config.data_nodes.push_back(node0);
  }

  if (!data_node_1.empty()) {
    DataNodeConfig node1;
    node1.address = data_node_1;
    node1.shard_id = 1;
    config.data_nodes.push_back(node1);
  }

  if (config.data_nodes.empty()) {
    std::cerr << "[ERROR] No data nodes configured. Please set DATA_NODE_0 "
              << "and/or DATA_NODE_1 environment variables." << std::endl;
    return EXIT_FAILURE;
  }

  // Create and initialize GatewayServer
  try {
    g_gateway_server = std::make_unique<GatewayServer>(config);

    std::cout << "[INFO] Initializing gateway server..." << std::endl;
    if (!g_gateway_server->initialize()) {
      std::cerr << "[ERROR] Failed to initialize gateway server" << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "[INFO] Gateway server initialized successfully!" << std::endl;
    std::cout << "[INFO] HTTP API available at http://0.0.0.0:" << http_port
              << std::endl;
    std::cout << "[INFO] Endpoint: POST /api/findAddress" << std::endl;
    std::cout << "[INFO] Press Ctrl+C to shutdown\n" << std::endl;

    // Start the HTTP server (blocking call)
    g_gateway_server->run();

    std::cout << "\n[INFO] Gateway server shutting down gracefully..."
              << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "[ERROR] Unhandled exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[INFO] Gateway server stopped." << std::endl;
  return EXIT_SUCCESS;
}
