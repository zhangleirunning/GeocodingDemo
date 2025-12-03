#ifndef GATEWAY_SERVER_H
#define GATEWAY_SERVER_H

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <crow.h>
#include <grpcpp/grpcpp.h>

#include "data_node.grpc.pb.h"

// Configuration for a single data node endpoint
struct DataNodeConfig {
  std::string address;  // e.g., "localhost:50051"
  int shard_id;
};

// Configuration for the gateway server
struct GatewayConfig {
  int http_port;                          // HTTP server port (default: 18080)
  std::vector<DataNodeConfig> data_nodes; // List of data node endpoints
  int grpc_timeout_ms;                    // gRPC call timeout in milliseconds
};

// Result from a single data node
struct DataNodeResult {
  int shard_id;
  bool success;
  std::string error_message;
  std::vector<datanode::AddressRecord> records;
};

// Scored address record for ranking
struct ScoredAddressRecord {
  datanode::AddressRecord record;
  int shard_id;
  double relevance_score;

  // For sorting by relevance (descending)
  bool operator<(const ScoredAddressRecord& other) const {
    return relevance_score > other.relevance_score;
  }
};

class GatewayServer {
 public:
  // Constructor with configuration
  explicit GatewayServer(const GatewayConfig& config);

  // Destructor
  ~GatewayServer();

  // Initialize the gateway server (create gRPC channels)
  bool initialize();

  // Start the HTTP server (blocking call)
  void run();

  // Shutdown the server gracefully
  void shutdown();

  // Check if shutdown has been requested
  bool isShutdownRequested() const;

 private:
  // Configuration
  GatewayConfig config_;

  // Crow HTTP server
  crow::SimpleApp app_;

  // gRPC channels and stubs for each data node (connection pooling)
  struct DataNodeConnection {
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<datanode::DataNodeService::Stub> stub;
    DataNodeConfig config;
  };
  std::vector<DataNodeConnection> connections_;

  // Shutdown flag
  std::atomic<bool> shutdown_requested_;

  // Setup HTTP routes
  void setupRoutes();

  // Query a single data node via gRPC
  DataNodeResult queryDataNode(const DataNodeConnection& connection,
                                const std::vector<std::string>& query_terms);

  // Query all data nodes in parallel
  std::vector<DataNodeResult> queryAllDataNodes(
      const std::vector<std::string>& query_terms);

  // Aggregate and rank results from multiple data nodes
  std::vector<ScoredAddressRecord> aggregateAndRankResults(
      const std::vector<DataNodeResult>& results,
      const std::vector<std::string>& query_terms,
      size_t max_results = 5);

  // Calculate relevance score for an address record
  double calculateRelevanceScore(
      const datanode::AddressRecord& record,
      const std::vector<std::string>& query_terms);

  // Check if two address records are duplicates
  bool isDuplicate(const datanode::AddressRecord& a,
                   const datanode::AddressRecord& b);
};

#endif // GATEWAY_SERVER_H
