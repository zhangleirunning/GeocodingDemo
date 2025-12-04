#include "data_node/forward_index.h"

void ForwardIndex::insert(size_t id, const AddressRecord& record) {
  records_[id] = record;
}

std::optional<AddressRecord> ForwardIndex::get(size_t id) const {
  auto it = records_.find(id);
  if (it != records_.end()) {
    return it->second;
  }
  return std::nullopt;
}

bool ForwardIndex::contains(size_t id) const {
  return records_.find(id) != records_.end();
}

size_t ForwardIndex::getStorageSize() const {
  size_t total_size = 0;

  // Size of the unordered_map overhead
  total_size += sizeof(records_);

  // Size of each entry in the map
  for (const auto& [id, record] : records_) {
    // Size of the key (size_t)
    total_size += sizeof(size_t);

    // Size of the AddressRecord
    total_size += sizeof(AddressRecord);

    // Size of dynamic string content in AddressRecord
    total_size += record.number.capacity();
    total_size += record.street.capacity();
    total_size += record.unit.capacity();
    total_size += record.city.capacity();
    total_size += record.postcode.capacity();
    total_size += record.original_street.capacity();
    total_size += record.original_unit.capacity();
    total_size += record.original_city.capacity();
  }

  return total_size;
}

size_t ForwardIndex::getRecordCount() const {
  return records_.size();
}
