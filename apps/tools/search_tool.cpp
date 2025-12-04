// Quick search test

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "data_node.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

int main(int argc, char** argv) {
  std::string server_address = "localhost:50051";

  auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  auto stub = datanode::DataNodeService::NewStub(channel);

  datanode::SearchRequest request;
  request.add_query_terms("SALINAS");

  datanode::SearchResponse response;
  ClientContext context;

  Status status = stub->Search(&context, request, &response);

  if (status.ok()) {
    std::cout << "Found " << response.result_count() << " results" << std::endl;

    for (int i = 0; i < std::min(5, response.results_size()); ++i) {
      const auto& record = response.results(i);
      std::cout << "\nResult " << (i + 1) << ":" << std::endl;
      std::cout << "  Address: " << record.number() << " " << record.street()
                << ", " << record.city() << std::endl;
    }
  } else {
    std::cout << "RPC failed: " << status.error_message() << std::endl;
  }

  return 0;
}
