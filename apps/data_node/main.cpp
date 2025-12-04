// Data Node Server Entry Point with gRPC

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "data_node.grpc.pb.h"
#include "data_node/data_node.h"

// Global pointer for signal handling
std::unique_ptr<grpc::Server> g_server;
volatile sig_atomic_t g_shutdown_requested = 0;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
  std::cout << "\n[INFO] Received signal " << signal
            << ", initiating graceful shutdown..." << std::endl;
  g_shutdown_requested = 1;

  if (g_server) {
    g_server->Shutdown();
  }
}

// gRPC service implementation
class DataNodeServiceImpl final : public datanode::DataNodeService::Service {
 public:
  explicit DataNodeServiceImpl(std::shared_ptr<DataNode> node)
      : node_(node) {}

  grpc::Status Search(grpc::ServerContext* context,
                     const datanode::SearchRequest* request,
                     datanode::SearchResponse* response) override {
    try {
      // Extract query terms from request
      std::vector<std::string> query_terms;
      for (const auto& term : request->query_terms()) {
        query_terms.push_back(term);
      }

      // Log the search request
      std::cout << "[INFO] Search request received with " << query_terms.size()
                << " term(s): ";
      for (size_t i = 0; i < query_terms.size(); ++i) {
        std::cout << "\"" << query_terms[i] << "\"";
        if (i < query_terms.size() - 1) {
          std::cout << ", ";
        }
      }
      std::cout << std::endl;

      // Execute search
      std::vector<AddressRecord> results = node_->search(query_terms);

      // Convert results to protobuf format
      for (const auto& record : results) {
        datanode::AddressRecord* pb_record = response->add_results();
        pb_record->set_hash(record.hash);
        pb_record->set_longitude(record.longitude);
        pb_record->set_latitude(record.latitude);
        pb_record->set_number(record.number);
        pb_record->set_street(record.street);
        pb_record->set_unit(record.unit);
        pb_record->set_city(record.city);
        pb_record->set_postcode(record.postcode);
      }

      response->set_result_count(results.size());

      std::cout << "[INFO] Search completed, returning " << results.size()
                << " result(s)" << std::endl;

      return grpc::Status::OK;

    } catch (const std::exception& e) {
      std::cerr << "[ERROR] Exception during search: " << e.what() << std::endl;
      return grpc::Status(grpc::StatusCode::INTERNAL,
                         "Internal error during search");
    }
  }

  grpc::Status GetStatistics(grpc::ServerContext* context,
                            const datanode::StatisticsRequest* request,
                            datanode::StatisticsResponse* response) override {
    try {
      DataNode::Statistics stats = node_->getStatistics();

      response->set_total_records(stats.total_records);
      response->set_radix_tree_memory(stats.radix_tree_memory);
      response->set_forward_index_size(stats.forward_index_size);
      response->set_load_time_ms(stats.load_time.count());

      std::cout << "[INFO] Statistics request served" << std::endl;

      return grpc::Status::OK;

    } catch (const std::exception& e) {
      std::cerr << "[ERROR] Exception getting statistics: " << e.what()
                << std::endl;
      return grpc::Status(grpc::StatusCode::INTERNAL,
                         "Internal error getting statistics");
    }
  }

 private:
  std::shared_ptr<DataNode> node_;
};

// Get configuration from environment variables with defaults
int getShardId() {
  const char* env_shard_id = std::getenv("SHARD_ID");
  if (env_shard_id) {
    try {
      int shard_id = std::stoi(env_shard_id);
      if (shard_id < 0) {
        std::cerr << "[ERROR] SHARD_ID must be non-negative" << std::endl;
        return -1;
      }
      return shard_id;
    } catch (const std::exception& e) {
      std::cerr << "[ERROR] Invalid SHARD_ID: " << env_shard_id << std::endl;
      return -1;
    }
  }

  // Default to shard 0
  return 0;
}

std::string getDataFilePath(int shard_id) {
  const char* env_data_path = std::getenv("DATA_FILE_PATH");
  if (env_data_path) {
    return std::string(env_data_path);
  }

  // Default based on shard_id
  return "data/shard_" + std::to_string(shard_id) + "_data_demo.csv";
}

int getPort(int shard_id) {
  const char* env_port = std::getenv("GRPC_PORT");
  if (env_port) {
    try {
      return std::stoi(env_port);
    } catch (const std::exception& e) {
      std::cerr << "[WARNING] Invalid GRPC_PORT: " << env_port
                << ", using default" << std::endl;
    }
  }

  // Default: 50051 for shard 0, 50052 for shard 1, etc.
  return 50051 + shard_id;
}

void runServer(std::shared_ptr<DataNode> node, int port) {
  std::string server_address = "0.0.0.0:" + std::to_string(port);

  DataNodeServiceImpl service(node);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  grpc::ServerBuilder builder;

  // Listen on the given address without authentication
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  // Register the service
  builder.RegisterService(&service);

  // Build and start the server
  g_server = builder.BuildAndStart();

  if (!g_server) {
    std::cerr << "[ERROR] Failed to start gRPC server" << std::endl;
    return;
  }

  std::cout << "[INFO] gRPC server listening on " << server_address << std::endl;
  std::cout << "[INFO] Server ready to accept requests" << std::endl;
  std::cout << "[INFO] Press Ctrl+C to shutdown\n" << std::endl;

  // Wait for the server to shutdown
  g_server->Wait();

  std::cout << "[INFO] gRPC server stopped" << std::endl;
}

int main(int argc, char* argv[]) {
  std::cout << "========================================" << std::endl;
  std::cout << "Data Node gRPC Server" << std::endl;
  std::cout << "========================================\n" << std::endl;

  // Get configuration from environment variables or use defaults
  int shard_id = getShardId();
  if (shard_id < 0) {
    return EXIT_FAILURE;
  }

  std::string data_file_path = getDataFilePath(shard_id);
  int port = getPort(shard_id);

  std::cout << "[INFO] Starting Data Node with configuration:" << std::endl;
  std::cout << "  Shard ID: " << shard_id << std::endl;
  std::cout << "  Data file: " << data_file_path << std::endl;
  std::cout << "  gRPC port: " << port << "\n" << std::endl;

  // Set up signal handlers for graceful shutdown
  std::signal(SIGINT, signalHandler);   // Ctrl+C
  std::signal(SIGTERM, signalHandler);  // Termination signal

  // Create and initialize DataNode
  try {
    auto data_node = std::make_shared<DataNode>(shard_id, data_file_path);

    std::cout << "[INFO] Initializing data node..." << std::endl;
    if (!data_node->initialize()) {
      std::cerr << "[ERROR] Failed to initialize data node" << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << "\n[INFO] Data node initialized successfully!" << std::endl;

    // Print statistics
    DataNode::Statistics stats = data_node->getStatistics();
    std::cout << "\n=== Startup Statistics ===" << std::endl;
    std::cout << "Total records indexed: " << stats.total_records << std::endl;
    std::cout << "RadixTree memory usage: " << stats.radix_tree_memory
              << " bytes" << std::endl;
    std::cout << "ForwardIndex storage: " << stats.forward_index_size
              << " bytes" << std::endl;
    std::cout << "Initialization time: " << stats.load_time.count() << " ms"
              << std::endl;
    std::cout << "==========================\n" << std::endl;

    // Start gRPC server
    runServer(data_node, port);

    std::cout << "\n[INFO] Data node shutting down gracefully..." << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "[ERROR] Unhandled exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[INFO] Data node stopped." << std::endl;
  return EXIT_SUCCESS;
}
