// Simple test client for data node gRPC server

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "data_node.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class DataNodeClient {
 public:
  DataNodeClient(std::shared_ptr<Channel> channel)
      : stub_(datanode::DataNodeService::NewStub(channel)) {}

  void Search(const std::vector<std::string>& query_terms) {
    datanode::SearchRequest request;
    for (const auto& term : query_terms) {
      request.add_query_terms(term);
    }

    datanode::SearchResponse response;
    ClientContext context;

    Status status = stub_->Search(&context, request, &response);

    if (status.ok()) {
      std::cout << "Search successful! Found " << response.result_count()
                << " results:" << std::endl;

      for (int i = 0; i < response.results_size(); ++i) {
        const auto& record = response.results(i);
        std::cout << "\nResult " << (i + 1) << ":" << std::endl;
        std::cout << "  Coordinates: (" << record.longitude() << ", "
                  << record.latitude() << ")" << std::endl;
        std::cout << "  Address: " << record.number() << " " << record.street();
        if (!record.unit().empty()) {
          std::cout << " Unit " << record.unit();
        }
        std::cout << std::endl;
        std::cout << "  City: " << record.city() << std::endl;
        std::cout << "  Postcode: " << record.postcode() << std::endl;
      }
    } else {
      std::cout << "RPC failed: " << status.error_message() << std::endl;
    }
  }

  void GetStatistics() {
    datanode::StatisticsRequest request;
    datanode::StatisticsResponse response;
    ClientContext context;

    Status status = stub_->GetStatistics(&context, request, &response);

    if (status.ok()) {
      std::cout << "\n=== Node Statistics ===" << std::endl;
      std::cout << "Total records: " << response.total_records() << std::endl;
      std::cout << "RadixTree memory: " << response.radix_tree_memory()
                << " bytes" << std::endl;
      std::cout << "ForwardIndex size: " << response.forward_index_size()
                << " bytes" << std::endl;
      std::cout << "Load time: " << response.load_time_ms() << " ms" << std::endl;
      std::cout << "======================\n" << std::endl;
    } else {
      std::cout << "RPC failed: " << status.error_message() << std::endl;
    }
  }

 private:
  std::unique_ptr<datanode::DataNodeService::Stub> stub_;
};

int main(int argc, char** argv) {
  std::string server_address = "localhost:50051";

  if (argc > 1) {
    server_address = argv[1];
  }

  std::cout << "Connecting to data node at " << server_address << std::endl;

  DataNodeClient client(
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

  // Test 1: Get statistics
  std::cout << "\n=== Test 1: Get Statistics ===" << std::endl;
  client.GetStatistics();

  // Test 2: Search for an address
  std::cout << "\n=== Test 2: Search for '3RD STREET' ===" << std::endl;
  client.Search({"3RD", "STREET"});

  return 0;
}
