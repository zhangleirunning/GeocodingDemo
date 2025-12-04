#ifndef DATA_NODE_RADIX_TREE_INDEX_H_
#define DATA_NODE_RADIX_TREE_INDEX_H_

#include <memory>
#include <string>
#include <vector>

class RadixTreeIndex {
 public:
  RadixTreeIndex();

  // Insert a term associated with an address ID
  void insert(const std::string& term, size_t address_id);

  // Search for all address IDs matching the prefix
  std::vector<size_t> search(const std::string& prefix) const;

  // Get memory usage statistics
  size_t getMemoryUsage() const;

  // Get total number of indexed terms
  size_t getTermCount() const;

 private:
  struct RadixNode {
    std::string edge_label;
    std::vector<size_t> address_ids;
    std::vector<std::unique_ptr<RadixNode>> children;

    RadixNode() = default;
    explicit RadixNode(const std::string& label) : edge_label(label) {}
  };

  std::unique_ptr<RadixNode> root_;
  size_t term_count_;

  void insertHelper(RadixNode* node,
                    const std::string& term,
                    size_t address_id,
                    size_t depth);
  void searchHelper(const RadixNode* node,
                    const std::string& prefix,
                    std::vector<size_t>& results,
                    size_t depth) const;
  void collectAllIds(const RadixNode* node,
                     std::vector<size_t>& results) const;
  size_t getMemoryUsageHelper(const RadixNode* node) const;
};

#endif  // DATA_NODE_RADIX_TREE_INDEX_H_
