#ifndef DATA_NODE_FORWARD_INDEX_H_
#define DATA_NODE_FORWARD_INDEX_H_

#include <optional>
#include <string>
#include <unordered_map>

#include "data_node/address_record.h"

class ForwardIndex {
 public:
  ForwardIndex() = default;

  // Store an address record
  void insert(const std::string& id, const AddressRecord& record);

  // Retrieve an address record by ID
  std::optional<AddressRecord> get(const std::string& id) const;

  // Check if an ID exists
  bool contains(const std::string& id) const;

  // Get total storage size (approximate bytes)
  size_t getStorageSize() const;

  // Get total number of records
  size_t getRecordCount() const;

 private:
  std::unordered_map<std::string, AddressRecord> records_;
};

#endif  // DATA_NODE_FORWARD_INDEX_H_
