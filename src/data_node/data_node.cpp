#include "data_node/data_node.h"

#include <algorithm>
#include <iostream>
#include <sstream>
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

std::vector<std::string> DataNode::generateSearchKeys(const AddressRecord& record) {
  std::vector<std::string> keys;

  // Normalize all components
  std::string norm_number = normalizer_->normalize(record.number);
  std::string norm_street = normalizer_->normalize(record.street);
  std::string norm_city = normalizer_->normalize(record.city);
  std::string norm_postcode = normalizer_->normalize(record.postcode);

  // Generate composite keys with different combinations
  // Key 1: number + separator + street + separator + city
  if (!norm_number.empty() && !norm_street.empty() && !norm_city.empty()) {
    std::string key1 = norm_number + KEY_SEPARATOR + norm_street + KEY_SEPARATOR + norm_city;
    keys.push_back(key1);
  }

  // Key 2: number + separator + street
  if (!norm_number.empty() && !norm_street.empty()) {
    std::string key2 = norm_number + KEY_SEPARATOR + norm_street;
    keys.push_back(key2);
  }

  // Key 3: number + separator + street + separator + city + separator + postcode
  if (!norm_number.empty() && !norm_street.empty() && !norm_city.empty() && !norm_postcode.empty()) {
    std::string key3 = norm_number + KEY_SEPARATOR + norm_street + KEY_SEPARATOR +
                       norm_city + KEY_SEPARATOR + norm_postcode;
    keys.push_back(key3);
  }

  return keys;
}

DataNode::ParsedAddress DataNode::parseQuery(const std::string& query) {
  ParsedAddress parsed;

  // Simple parser: split by comma and whitespace
  // Expected format: "number street, city, postcode" or variations
  std::vector<std::string> parts;
  std::string current;

  for (char c : query) {
    if (c == ',') {
      if (!current.empty()) {
        parts.push_back(current);
        current.clear();
      }
    } else {
      current += c;
    }
  }
  if (!current.empty()) {
    parts.push_back(current);
  }

  // Trim whitespace from parts
  for (auto& part : parts) {
    // Trim leading whitespace
    size_t start = 0;
    while (start < part.length() && std::isspace(static_cast<unsigned char>(part[start]))) {
      start++;
    }
    // Trim trailing whitespace
    size_t end = part.length();
    while (end > start && std::isspace(static_cast<unsigned char>(part[end - 1]))) {
      end--;
    }
    part = part.substr(start, end - start);
  }

  // Parse based on number of parts
  if (parts.size() >= 1) {
    // First part should contain number and street
    std::string first_part = parts[0];
    std::istringstream iss(first_part);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
      tokens.push_back(token);
    }

    // First token is likely the number
    if (!tokens.empty()) {
      parsed.number = tokens[0];

      // Rest is the street
      if (tokens.size() > 1) {
        for (size_t i = 1; i < tokens.size(); ++i) {
          if (i > 1) parsed.street += " ";
          parsed.street += tokens[i];
        }
      }
    }
  }

  if (parts.size() >= 2) {
    parsed.city = parts[1];
  }

  if (parts.size() >= 3) {
    parsed.postcode = parts[2];
  }

  return parsed;
}

void DataNode::buildIndexes(const std::vector<AddressRecord>& records) {
  std::cout << "[INFO] [DataNode] Building indexes for " << records.size()
            << " records..." << std::endl;

  for (const AddressRecord& record : records) {
    // Use the hash field as the unique ID
    size_t record_id = record.hash;

    // Insert into ForwardIndex
    forward_index_->insert(record_id, record);

    // Generate and insert composite search keys for structured queries
    std::vector<std::string> search_keys = generateSearchKeys(record);
    for (const auto& key : search_keys) {
      radix_index_->insert(key, record_id);
    }

    // Also index individual fields for backward compatibility and partial matching
    // This allows searching by individual terms like "STREET" or "SEATTLE"
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

    if (!record.number.empty()) {
      std::string normalized_number = normalizer_->normalize(record.number);
      radix_index_->insert(normalized_number, record_id);
    }
  }

  std::cout << "[INFO] [DataNode] Indexes built successfully" << std::endl;
}

std::vector<size_t> DataNode::findMatchingIds(
    const std::vector<std::string>& query_terms) {
  if (query_terms.empty()) {
    return {};
  }

  // Check if this is a single query string that looks like a full address
  // (contains comma, suggesting it's a structured address query)
  if (query_terms.size() == 1 && query_terms[0].find(',') != std::string::npos) {
    // Parse the query as a structured address
    ParsedAddress parsed = parseQuery(query_terms[0]);

    // Generate search keys from the parsed address
    std::vector<std::string> search_keys;

    // Normalize components
    std::string norm_number = normalizer_->normalize(parsed.number);
    std::string norm_street = normalizer_->normalize(parsed.street);
    std::string norm_city = normalizer_->normalize(parsed.city);
    std::string norm_postcode = normalizer_->normalize(parsed.postcode);

    // Try most specific key first (with postcode)
    if (!norm_number.empty() && !norm_street.empty() && !norm_city.empty() && !norm_postcode.empty()) {
      std::string key = norm_number + KEY_SEPARATOR + norm_street + KEY_SEPARATOR +
                       norm_city + KEY_SEPARATOR + norm_postcode;
      search_keys.push_back(key);
    }

    // Try key with city
    if (!norm_number.empty() && !norm_street.empty() && !norm_city.empty()) {
      std::string key = norm_number + KEY_SEPARATOR + norm_street + KEY_SEPARATOR + norm_city;
      search_keys.push_back(key);
    }

    // Try key without city
    if (!norm_number.empty() && !norm_street.empty()) {
      std::string key = norm_number + KEY_SEPARATOR + norm_street;
      search_keys.push_back(key);
    }

    // Search with each key and return first match
    for (const auto& key : search_keys) {
      std::vector<size_t> results = radix_index_->search(key);
      if (!results.empty()) {
        std::cout << "[INFO] [DataNode] Found " << results.size()
                  << " matches using key: " << key << std::endl;
        return results;
      }
    }

    // No matches found with composite keys
    return {};
  }

  // Original logic for multi-term queries
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
