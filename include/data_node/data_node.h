#ifndef DATA_NODE_DATA_NODE_H_
#define DATA_NODE_DATA_NODE_H_

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "data_node/address_normalizer.h"
#include "data_node/address_record.h"
#include "data_node/forward_index.h"
#include "data_node/radix_tree_index.h"

class DataNode {
 public:
  // Statistics structure for reporting node metrics
  struct Statistics {
    size_t total_records;
    size_t radix_tree_memory;
    size_t forward_index_size;
    std::chrono::milliseconds load_time;
  };

  // Initialize with shard configuration
  DataNode(int shard_id, const std::string& data_file_path);

  // Load data and build indexes
  bool initialize();

  // Search for addresses matching query terms
  std::vector<AddressRecord> search(const std::vector<std::string>& query_terms);

  // Get node statistics
  Statistics getStatistics() const;

 private:
  int shard_id_;
  std::string data_file_path_;

  std::unique_ptr<RadixTreeIndex> radix_index_;
  std::unique_ptr<ForwardIndex> forward_index_;
  std::unique_ptr<AddressNormalizer> normalizer_;

  Statistics stats_;

  void buildIndexes(const std::vector<AddressRecord>& records);
  std::vector<size_t> findMatchingIds(
      const std::vector<std::string>& query_terms);
};

#endif  // DATA_NODE_DATA_NODE_H_
