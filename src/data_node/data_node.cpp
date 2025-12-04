#include "data_node/data_node.h"

#include <algorithm>
#include <iostream>
#include <unordered_set>

#include "data_node/address_normalizer.h"
#include "data_node/csv_parser.h"
#include "data_node/forward_index.h"
#include "data_node/radix_tree_index.h"

DataNode::DataNode(int shard_id, const std::string& data_file_path)
    : shard_id_(shard_id),
      data_file_path_(data_file_path),
      radix_index_(std::make_unique<RadixTreeIndex>()),
      forward_index_(std::make_unique<ForwardIndex>()),
      normalizer_(std::make_unique<AddressNormalizer>()) {
  stats_.total_records = 0;
  stats_.radix_tree_memory = 0;
  stats_.forward_index_size = 0;
  stats_.load_time = std::chrono::milliseconds(0);
}

bool DataNode::initialize() {
  auto start_time = std::chrono::steady_clock::now();

  std::cout << "[INFO] [DataNode] Starting data load from: " << data_file_path_
            << " (shard_id=" << shard_id_ << ")" << std::endl;

  try {
    // Parse CSV file
    CSVParser parser;
    std::vector<AddressRecord> records = parser.parse(data_file_path_);

    if (records.empty()) {
      std::cerr << "[ERROR] [DataNode] No valid records loaded from "
                << data_file_path_ << std::endl;
      return false;
    }

    std::cout << "[INFO] [DataNode] Successfully parsed " << records.size()
              << " records (errors: " << parser.getErrorCount() << ")"
              << std::endl;

    // Build indexes
    buildIndexes(records);

    // Calculate statistics
    auto end_time = std::chrono::steady_clock::now();
    stats_.total_records = records.size();
    stats_.radix_tree_memory = radix_index_->getMemoryUsage();
    stats_.forward_index_size = forward_index_->getStorageSize();
    stats_.load_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                               start_time);

    std::cout << "[INFO] [DataNode] Index building complete:" << std::endl;
    std::cout << "  - Total records: " << stats_.total_records << std::endl;
    std::cout << "  - RadixTree memory: " << stats_.radix_tree_memory
              << " bytes" << std::endl;
    std::cout << "  - ForwardIndex size: " << stats_.forward_index_size
              << " bytes" << std::endl;
    std::cout << "  - Load time: " << stats_.load_time.count() << " ms"
              << std::endl;

    return true;
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] [DataNode] Exception during initialization: "
              << e.what() << std::endl;
    return false;
  }
}

void DataNode::buildIndexes(const std::vector<AddressRecord>& records) {
  std::cout << "[INFO] [DataNode] Building indexes for " << records.size()
            << " records..." << std::endl;

  for (const AddressRecord& record : records) {
    // Use the hash field as the unique ID
    size_t record_id = record.hash;

    // Insert into ForwardIndex
    forward_index_->insert(record_id, record);

    // Insert normalized searchable fields into RadixTreeIndex
    // Only insert non-empty fields
    if (!record.street.empty()) {
      std::string normalized_street = normalizer_->normalize(record.street);
      radix_index_->insert(normalized_street, record_id);
    }

    if (!record.city.empty()) {
      std::string normalized_city = normalizer_->normalize(record.city);
      radix_index_->insert(normalized_city, record_id);
    }

    if (!record.postcode.empty()) {
      std::string normalized_postcode = normalizer_->normalize(record.postcode);
      radix_index_->insert(normalized_postcode, record_id);
    }

    // Note: The current AddressRecord doesn't have district and region fields
    // as shown in the design document, so we skip those
  }

  std::cout << "[INFO] [DataNode] Indexes built successfully" << std::endl;
}

std::vector<size_t> DataNode::findMatchingIds(
    const std::vector<std::string>& query_terms) {
  if (query_terms.empty()) {
    return {};
  }

  // Normalize query terms
  std::vector<std::string> normalized_terms;
  for (const auto& term : query_terms) {
    normalized_terms.push_back(normalizer_->normalize(term));
  }

  // Find IDs matching the first term
  std::vector<size_t> first_term_ids =
      radix_index_->search(normalized_terms[0]);

  if (first_term_ids.empty() || normalized_terms.size() == 1) {
    return first_term_ids;
  }

  // Convert to set for efficient intersection
  std::unordered_set<size_t> result_ids(first_term_ids.begin(),
                                         first_term_ids.end());

  // Intersect with results from remaining terms
  for (size_t i = 1; i < normalized_terms.size(); ++i) {
    std::vector<size_t> term_ids =
        radix_index_->search(normalized_terms[i]);
    std::unordered_set<size_t> term_id_set(term_ids.begin(),
                                            term_ids.end());

    // Keep only IDs that appear in both sets
    std::unordered_set<size_t> intersection;
    for (const auto& id : result_ids) {
      if (term_id_set.count(id) > 0) {
        intersection.insert(id);
      }
    }

    result_ids = std::move(intersection);

    if (result_ids.empty()) {
      break;  // No matches, can stop early
    }
  }

  // Convert back to vector
  return std::vector<size_t>(result_ids.begin(), result_ids.end());
}

std::vector<AddressRecord> DataNode::search(
    const std::vector<std::string>& query_terms) {
  try {
    std::cout << "[INFO] [DataNode] Processing search query with "
              << query_terms.size() << " terms" << std::endl;

    if (query_terms.empty()) {
      std::cout << "[INFO] [DataNode] Empty query, returning 0 results"
                << std::endl;
      return {};
    }

    // Find matching IDs using RadixTreeIndex
    std::vector<size_t> matching_ids = findMatchingIds(query_terms);

    std::cout << "[INFO] [DataNode] Found " << matching_ids.size()
              << " matching IDs" << std::endl;

    // Retrieve complete records from ForwardIndex
    std::vector<AddressRecord> results;
    for (const auto& id : matching_ids) {
      std::optional<AddressRecord> record = forward_index_->get(id);
      if (record.has_value()) {
        results.push_back(record.value());
      } else {
        std::cerr << "[WARNING] [DataNode] Index inconsistency: ID " << id
                  << " found in RadixTree but not in ForwardIndex" << std::endl;
      }
    }

    std::cout << "[INFO] [DataNode] Returning " << results.size()
              << " complete records" << std::endl;

    return results;
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] [DataNode] Exception during query processing: "
              << e.what() << std::endl;
    return {};  // Return empty result on exception
  }
}

DataNode::Statistics DataNode::getStatistics() const { return stats_; }
