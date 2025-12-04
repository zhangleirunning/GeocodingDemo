#include "gateway/gateway_server.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <sstream>

#include "data_node/address_normalizer.h"

GatewayServer::GatewayServer(const GatewayConfig& config)
    : config_(config), shutdown_requested_(false) {
  std::cout << "[INFO] GatewayServer created with configuration:" << std::endl;
  std::cout << "  HTTP Port: " << config_.http_port << std::endl;
  std::cout << "  Data Nodes: " << config_.data_nodes.size() << std::endl;
  std::cout << "  gRPC Timeout: " << config_.grpc_timeout_ms << " ms"
            << std::endl;
}

GatewayServer::~GatewayServer() {
  std::cout << "[INFO] GatewayServer destructor called" << std::endl;
}

bool GatewayServer::initialize() {
  std::cout << "[INFO] Initializing gateway server..." << std::endl;

  // Create gRPC channels and stubs for each data node
  for (const auto& node_config : config_.data_nodes) {
    DataNodeConnection connection;
    connection.config = node_config;

    // Create gRPC channel with insecure credentials
    connection.channel = grpc::CreateChannel(
        node_config.address, grpc::InsecureChannelCredentials());

    if (!connection.channel) {
      std::cerr << "[ERROR] Failed to create gRPC channel for data node "
                << node_config.shard_id << " at " << node_config.address
                << std::endl;
      return false;
    }

    // Create stub for the data node service
    connection.stub = datanode::DataNodeService::NewStub(connection.channel);

    if (!connection.stub) {
      std::cerr << "[ERROR] Failed to create gRPC stub for data node "
                << node_config.shard_id << std::endl;
      return false;
    }

    connections_.push_back(std::move(connection));

    std::cout << "[INFO] Created gRPC connection to data node "
              << node_config.shard_id << " at " << node_config.address
              << std::endl;
  }

  // Setup HTTP routes
  setupRoutes();

  std::cout << "[INFO] Gateway server initialized successfully" << std::endl;
  return true;
}

void GatewayServer::setupRoutes() {
  // Health check endpoint
  CROW_ROUTE(app_, "/health")
  ([this]() {
    crow::json::wvalue response;
    response["status"] = "healthy";
    response["data_nodes"] = connections_.size();
    return response;
  });

  // Serve web frontend HTML at root
  CROW_ROUTE(app_, "/")
  ([]() {
    // Read and serve the web frontend HTML file
    std::ifstream file("/app/web/index.html");
    if (!file.is_open()) {
      // Fallback to API info if web frontend not found
      crow::json::wvalue response;
      response["service"] = "Geocoding Gateway";
      response["version"] = "1.0.0";
      response["endpoints"] = crow::json::wvalue::list({"/health",
                                                         "/api/findAddress"});
      return crow::response(response);
    }

    std::string html_content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    file.close();

    crow::response res(html_content);
    res.set_header("Content-Type", "text/html");
    return res;
  });

  // Find address endpoint
  CROW_ROUTE(app_, "/api/findAddress")
      .methods(crow::HTTPMethod::POST)([this](const crow::request& req) {
        try {
          // Parse JSON request
          auto json_body = crow::json::load(req.body);
          if (!json_body) {
            crow::json::wvalue error_response;
            error_response["error"] = "Invalid JSON in request body";
            return crow::response(400, error_response);
          }

          // Extract address keyword
          if (!json_body.has("address")) {
            crow::json::wvalue error_response;
            error_response["error"] =
                "Missing 'address' field in request body";
            return crow::response(400, error_response);
          }

          std::string address_keyword = json_body["address"].s();

          // Validate that address keyword is not empty
          if (address_keyword.empty()) {
            crow::json::wvalue error_response;
            error_response["error"] = "Address keyword cannot be empty";
            return crow::response(400, error_response);
          }

          std::cout << "[INFO] Received findAddress request: \""
                    << address_keyword << "\"" << std::endl;

          std::vector<std::string> query_terms;

          // Check if this is a structured address query (contains comma)
          if (address_keyword.find(',') != std::string::npos) {
            // Structured address query - pass as single term to preserve structure
            // The DataNode will parse it into components
            std::cout << "[INFO] Detected structured address query" << std::endl;
            query_terms.push_back(address_keyword);
          } else {
            // Traditional multi-term query - split by whitespace
            std::cout << "[INFO] Detected traditional multi-term query" << std::endl;
            std::istringstream iss(address_keyword);
            std::string term;
            while (iss >> term) {
              query_terms.push_back(term);
            }
          }

          if (query_terms.empty()) {
            crow::json::wvalue error_response;
            error_response["error"] =
                "Address keyword must contain at least one term";
            return crow::response(400, error_response);
          }

          std::cout << "[INFO] Query terms: ";
          for (size_t i = 0; i < query_terms.size(); ++i) {
            std::cout << "\"" << query_terms[i] << "\"";
            if (i < query_terms.size() - 1) {
              std::cout << ", ";
            }
          }
          std::cout << std::endl;

          // Query all data nodes
          auto results = queryAllDataNodes(query_terms);

          // Count successful and failed nodes
          int successful_nodes = 0;
          int failed_nodes = 0;
          for (const auto& result : results) {
            if (result.success) {
              successful_nodes++;
            } else {
              failed_nodes++;
              std::cerr << "[WARNING] Data node " << result.shard_id
                        << " failed: " << result.error_message << std::endl;
            }
          }

          // Aggregate and rank results (top 5)
          auto ranked_results = aggregateAndRankResults(results, query_terms, 5);

          // Build JSON response
          crow::json::wvalue response;
          response["query"] = address_keyword;

          // Build query_terms array
          std::vector<crow::json::wvalue> terms_array;
          for (const auto& term : query_terms) {
            terms_array.push_back(term);
          }
          response["query_terms"] = std::move(terms_array);

          // Build results array with ranked records
          std::vector<crow::json::wvalue> results_array;
          for (const auto& scored : ranked_results) {
            crow::json::wvalue json_record;
            json_record["hash"] = scored.record.hash();
            json_record["longitude"] = scored.record.longitude();
            json_record["latitude"] = scored.record.latitude();
            json_record["number"] = scored.record.number();
            json_record["street"] = scored.record.street();
            json_record["unit"] = scored.record.unit();
            json_record["city"] = scored.record.city();
            json_record["postcode"] = scored.record.postcode();
            json_record["shard_id"] = scored.shard_id;
            json_record["relevance_score"] = scored.relevance_score;
            results_array.push_back(std::move(json_record));
          }

          response["results"] = std::move(results_array);
          response["result_count"] = static_cast<int>(ranked_results.size());
          response["successful_nodes"] = successful_nodes;
          response["failed_nodes"] = failed_nodes;

          std::cout << "[INFO] Returning " << ranked_results.size()
                    << " ranked result(s) from " << successful_nodes
                    << " successful node(s)" << std::endl;

          // Return 200 OK with results (even if empty)
          // Return 207 Multi-Status if some nodes failed but we have results
          // Return 503 Service Unavailable if all nodes failed
          if (failed_nodes > 0 && successful_nodes == 0) {
            response["error"] = "All data nodes failed to respond";
            return crow::response(503, response);
          } else if (failed_nodes > 0) {
            return crow::response(207, response);
          } else {
            return crow::response(200, response);
          }

        } catch (const std::exception& e) {
          std::cerr << "[ERROR] Exception in findAddress endpoint: " << e.what()
                    << std::endl;
          crow::json::wvalue error_response;
          error_response["error"] = "Internal server error";
          error_response["details"] = e.what();
          return crow::response(500, error_response);
        }
      });

  std::cout << "[INFO] HTTP routes configured" << std::endl;
}

DataNodeResult GatewayServer::queryDataNode(
    const DataNodeConnection& connection,
    const std::vector<std::string>& query_terms) {
  DataNodeResult result;
  result.shard_id = connection.config.shard_id;
  result.success = false;

  // Start timing the gRPC call
  auto start_time = std::chrono::steady_clock::now();

  try {
    // Create gRPC context with timeout
    grpc::ClientContext context;
    auto deadline = std::chrono::system_clock::now() +
                    std::chrono::milliseconds(config_.grpc_timeout_ms);
    context.set_deadline(deadline);

    std::cout << "[INFO] Starting gRPC call to data node "
              << connection.config.shard_id << " at "
              << connection.config.address << " (timeout: "
              << config_.grpc_timeout_ms << "ms)" << std::endl;

    // Prepare request
    datanode::SearchRequest request;
    for (const auto& term : query_terms) {
      request.add_query_terms(term);
    }

    // Make gRPC call
    datanode::SearchResponse response;
    grpc::Status status = connection.stub->Search(&context, request, &response);

    // Calculate elapsed time
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          end_time - start_time)
                          .count();

    if (status.ok()) {
      result.success = true;
      // Copy results
      for (const auto& record : response.results()) {
        result.records.push_back(record);
      }

      std::cout << "[INFO] Data node " << connection.config.shard_id
                << " returned " << result.records.size() << " result(s) in "
                << elapsed_ms << "ms" << std::endl;
    } else {
      // Check if it was a timeout
      if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED) {
        result.error_message =
            "gRPC timeout after " + std::to_string(elapsed_ms) + "ms";
        std::cerr << "[ERROR] Data node " << connection.config.shard_id
                  << " query timed out after " << elapsed_ms << "ms"
                  << std::endl;
      } else {
        result.error_message = "gRPC error: " + status.error_message() +
                               " (code: " +
                               std::to_string(status.error_code()) + ")";
        std::cerr << "[ERROR] Data node " << connection.config.shard_id
                  << " query failed after " << elapsed_ms
                  << "ms: " << status.error_message()
                  << " (code: " << status.error_code() << ")" << std::endl;
      }
    }

  } catch (const std::exception& e) {
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          end_time - start_time)
                          .count();

    result.error_message = std::string("Exception: ") + e.what();
    std::cerr << "[ERROR] Exception querying data node "
              << connection.config.shard_id << " after " << elapsed_ms
              << "ms: " << e.what() << std::endl;
  }

  return result;
}

std::vector<DataNodeResult> GatewayServer::queryAllDataNodes(
    const std::vector<std::string>& query_terms) {
  std::cout << "[INFO] Querying " << connections_.size()
            << " data node(s) in parallel..." << std::endl;

  // Start timing the overall parallel query operation
  auto overall_start = std::chrono::steady_clock::now();

  // Launch async queries to all data nodes using std::async
  // std::launch::async ensures each call runs in a separate thread
  std::vector<std::future<DataNodeResult>> futures;
  futures.reserve(connections_.size());

  for (const auto& connection : connections_) {
    std::cout << "[INFO] Launching async gRPC call to data node "
              << connection.config.shard_id << std::endl;

    futures.push_back(std::async(std::launch::async,
                                 &GatewayServer::queryDataNode, this,
                                 std::cref(connection), std::cref(query_terms)));
  }

  std::cout << "[INFO] All " << futures.size()
            << " async gRPC calls launched, waiting for results..." << std::endl;

  // Collect results from all futures
  std::vector<DataNodeResult> results;
  results.reserve(futures.size());

  int successful_count = 0;
  int failed_count = 0;
  int timeout_count = 0;

  for (size_t i = 0; i < futures.size(); ++i) {
    try {
      // Wait for the future to complete and get the result
      // Note: future.get() will block until the result is available
      // The timeout is handled within queryDataNode via gRPC context deadline
      DataNodeResult result = futures[i].get();

      results.push_back(result);

      if (result.success) {
        successful_count++;
      } else {
        failed_count++;
        // Check if it was a timeout
        if (result.error_message.find("timeout") != std::string::npos ||
            result.error_message.find("DEADLINE_EXCEEDED") !=
                std::string::npos) {
          timeout_count++;
        }
      }

    } catch (const std::exception& e) {
      std::cerr << "[ERROR] Exception collecting result from future " << i
                << ": " << e.what() << std::endl;

      // Add a failed result for this node
      DataNodeResult failed_result;
      failed_result.shard_id = (i < connections_.size())
                                   ? connections_[i].config.shard_id
                                   : -1;
      failed_result.success = false;
      failed_result.error_message =
          std::string("Future exception: ") + e.what();
      results.push_back(failed_result);
      failed_count++;
    }
  }

  // Calculate overall elapsed time
  auto overall_end = std::chrono::steady_clock::now();
  auto overall_elapsed_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(overall_end -
                                                            overall_start)
          .count();

  // Log performance metrics
  std::cout << "[INFO] Parallel query completed in " << overall_elapsed_ms
            << "ms" << std::endl;
  std::cout << "[INFO] Results summary: " << successful_count
            << " successful, " << failed_count << " failed";
  if (timeout_count > 0) {
    std::cout << " (" << timeout_count << " timeouts)";
  }
  std::cout << std::endl;

  // Log partial failure warning if applicable
  if (failed_count > 0 && successful_count > 0) {
    std::cout << "[WARNING] Partial failure: " << successful_count
              << " node(s) succeeded but " << failed_count << " node(s) failed"
              << std::endl;
  } else if (failed_count > 0 && successful_count == 0) {
    std::cerr << "[ERROR] Complete failure: all " << failed_count
              << " data node(s) failed to respond" << std::endl;
  }

  return results;
}

void GatewayServer::run() {
  std::cout << "[INFO] Starting HTTP server on port " << config_.http_port
            << "..." << std::endl;

  // Configure Crow to use the specified port
  app_.port(config_.http_port);

  // Run the server (blocking call)
  // Note: Crow's run() is blocking, so this will block until shutdown
  try {
    app_.run();
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] Exception running HTTP server: " << e.what()
              << std::endl;
  }

  std::cout << "[INFO] HTTP server stopped" << std::endl;
}

void GatewayServer::shutdown() {
  std::cout << "[INFO] Shutting down gateway server..." << std::endl;
  shutdown_requested_ = true;

  // Stop the Crow HTTP server
  app_.stop();

  std::cout << "[INFO] Gateway server shutdown complete" << std::endl;
}

bool GatewayServer::isShutdownRequested() const {
  return shutdown_requested_.load();
}

bool GatewayServer::isDuplicate(const datanode::AddressRecord& a,
                                 const datanode::AddressRecord& b) {
  // Two addresses are considered duplicates if they have the same:
  // - Street number
  // - Street name
  // - City
  // - Postcode
  // We ignore unit numbers as they might differ in formatting

  return a.number() == b.number() &&
         a.street() == b.street() &&
         a.city() == b.city() &&
         a.postcode() == b.postcode();
}

double GatewayServer::calculateRelevanceScore(
    const datanode::AddressRecord& record,
    const std::vector<std::string>& query_terms) {
  double score = 0.0;

  // Collect all searchable fields from the record
  std::vector<std::string> fields = {
    record.street(),
    record.city(),
    record.postcode(),
    record.number()
  };

  // Count how many query terms match in the record
  int matching_terms = 0;
  for (const auto& term : query_terms) {
    bool found = false;
    for (const auto& field : fields) {
      if (field.find(term) != std::string::npos) {
        found = true;
        break;
      }
    }
    if (found) {
      matching_terms++;
    }
  }

  // Base score: percentage of query terms that match
  // This is the most important factor
  if (!query_terms.empty()) {
    score += (static_cast<double>(matching_terms) / query_terms.size()) * 100.0;
  }

  // Bonus points for position of matches in address fields
  // Street matches are most important, then city, then postcode
  for (const auto& term : query_terms) {
    if (record.street().find(term) != std::string::npos) {
      // Street match at beginning is worth more
      if (record.street().find(term) == 0) {
        score += 15.0;
      } else {
        score += 10.0;
      }
    }

    if (record.city().find(term) != std::string::npos) {
      // City match at beginning is worth more
      if (record.city().find(term) == 0) {
        score += 8.0;
      } else {
        score += 5.0;
      }
    }

    if (record.postcode().find(term) != std::string::npos) {
      score += 3.0;
    }

    if (record.number().find(term) != std::string::npos) {
      score += 5.0;
    }
  }

  // Bonus points for completeness of address data
  // More complete addresses are more useful
  int completeness = 0;
  if (!record.number().empty()) completeness++;
  if (!record.street().empty()) completeness++;
  if (!record.unit().empty()) completeness++;
  if (!record.city().empty()) completeness++;
  if (!record.postcode().empty()) completeness++;

  // Add up to 10 points for completeness (2 points per field)
  score += completeness * 2.0;

  return score;
}

std::vector<ScoredAddressRecord> GatewayServer::aggregateAndRankResults(
    const std::vector<DataNodeResult>& results,
    const std::vector<std::string>& query_terms,
    size_t max_results) {

  std::cout << "[INFO] Aggregating and ranking results..." << std::endl;

  // Collect all records with their scores
  std::vector<ScoredAddressRecord> scored_records;

  for (const auto& result : results) {
    if (!result.success) {
      continue;
    }

    for (const auto& record : result.records) {
      // Calculate relevance score
      double score = calculateRelevanceScore(record, query_terms);

      // Check if this is a duplicate of an existing record
      bool is_duplicate = false;
      for (auto& existing : scored_records) {
        if (isDuplicate(existing.record, record)) {
          is_duplicate = true;
          // Keep the one with higher score
          if (score > existing.relevance_score) {
            std::cout << "[INFO] Found duplicate address, keeping higher scored version (new score: "
                      << score << " vs old score: " << existing.relevance_score << ")" << std::endl;
            existing.record = record;
            existing.shard_id = result.shard_id;
            existing.relevance_score = score;
          } else {
            std::cout << "[INFO] Found duplicate address, keeping existing higher scored version" << std::endl;
          }
          break;
        }
      }

      if (!is_duplicate) {
        ScoredAddressRecord scored;
        scored.record = record;
        scored.shard_id = result.shard_id;
        scored.relevance_score = score;
        scored_records.push_back(scored);
      }
    }
  }

  std::cout << "[INFO] Total unique records after deduplication: "
            << scored_records.size() << std::endl;

  // Sort by relevance score (descending)
  std::sort(scored_records.begin(), scored_records.end());

  // Return top N results
  if (scored_records.size() > max_results) {
    std::cout << "[INFO] Limiting results to top " << max_results << std::endl;
    scored_records.resize(max_results);
  }

  // Log the top results with their scores
  std::cout << "[INFO] Top " << scored_records.size() << " results:" << std::endl;
  for (size_t i = 0; i < scored_records.size(); ++i) {
    const auto& scored = scored_records[i];
    std::cout << "  " << (i + 1) << ". Score: " << scored.relevance_score
              << " - " << scored.record.number() << " " << scored.record.street()
              << ", " << scored.record.city() << " " << scored.record.postcode()
              << " (shard " << scored.shard_id << ")" << std::endl;
  }

  return scored_records;
}
