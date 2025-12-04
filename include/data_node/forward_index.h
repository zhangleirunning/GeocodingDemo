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
  void insert(size_t id, const AddressRecord& record);

  // Retrieve an address record by ID
  std::optional<AddressRecord> get(size_t id) const;

  // Check if an ID exists
  bool contains(size_t id) const;

  // Get total storage size (approximate bytes)
  size_t getStorageSize() const;

  // Get total number of records
  size_t getRecordCount() const;

 private:
  std::unordered_map<size_t, AddressRecord> records_;
};

#endif  // DATA_NODE_FORWARD_INDEX_H_
