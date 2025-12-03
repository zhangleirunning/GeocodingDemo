#include "data_node/radix_tree_index.h"

#include <algorithm>

RadixTreeIndex::RadixTreeIndex() : root_(std::make_unique<RadixNode>()), term_count_(0) {}

void RadixTreeIndex::insert(const std::string& term, const std::string& address_id) {
  if (term.empty()) {
    return;
  }
  insertHelper(root_.get(), term, address_id, 0);
  term_count_++;
}

void RadixTreeIndex::insertHelper(RadixNode* node,
                                   const std::string& term,
                                   const std::string& address_id,
                                   size_t depth) {
  // If we've consumed the entire term, add the address_id to this node
  if (depth >= term.length()) {
    // Check if this address_id is already in the list
    if (std::find(node->address_ids.begin(), node->address_ids.end(), address_id) ==
        node->address_ids.end()) {
      node->address_ids.push_back(address_id);
      // Keep address_ids sorted for deterministic behavior
      std::sort(node->address_ids.begin(), node->address_ids.end());
    }
    return;
  }

  std::string remaining = term.substr(depth);

  // Find a child that shares a prefix with remaining
  for (auto& child : node->children) {
    const std::string& edge_label = child->edge_label;
    size_t common_prefix_len = 0;

    // Find the length of the common prefix
    while (common_prefix_len < edge_label.length() &&
           common_prefix_len < remaining.length() &&
           edge_label[common_prefix_len] == remaining[common_prefix_len]) {
      common_prefix_len++;
    }

    if (common_prefix_len > 0) {
      // We found a child with a common prefix
      if (common_prefix_len == edge_label.length()) {
        // The entire edge label matches, continue down this path
        insertHelper(child.get(), term, address_id, depth + common_prefix_len);
        return;
      } else {
        // Need to split the edge
        // Create a new intermediate node
        auto new_node = std::make_unique<RadixNode>(edge_label.substr(0, common_prefix_len));

        // Update the existing child's edge label
        std::string old_suffix = edge_label.substr(common_prefix_len);
        child->edge_label = old_suffix;

        // Move the old child under the new node
        new_node->children.push_back(std::move(child));

        // Replace the old child with the new node
        child = std::move(new_node);

        // Now continue insertion from the new node
        if (common_prefix_len == remaining.length()) {
          // The term ends at this split point
          if (std::find(child->address_ids.begin(), child->address_ids.end(), address_id) ==
              child->address_ids.end()) {
            child->address_ids.push_back(address_id);
            std::sort(child->address_ids.begin(), child->address_ids.end());
          }
        } else {
          // Create a new child for the remaining part of the term
          std::string new_suffix = remaining.substr(common_prefix_len);
          auto new_child = std::make_unique<RadixNode>(new_suffix);
          new_child->address_ids.push_back(address_id);
          child->children.push_back(std::move(new_child));
          // Keep children sorted by edge_label for deterministic traversal
          std::sort(child->children.begin(), child->children.end(),
                    [](const std::unique_ptr<RadixNode>& a, const std::unique_ptr<RadixNode>& b) {
                      return a->edge_label < b->edge_label;
                    });
        }
        return;
      }
    }
  }

  // No matching child found, create a new one
  auto new_child = std::make_unique<RadixNode>(remaining);
  new_child->address_ids.push_back(address_id);
  node->children.push_back(std::move(new_child));
  // Keep children sorted by edge_label for deterministic traversal
  std::sort(node->children.begin(), node->children.end(),
            [](const std::unique_ptr<RadixNode>& a, const std::unique_ptr<RadixNode>& b) {
              return a->edge_label < b->edge_label;
            });
}

std::vector<std::string> RadixTreeIndex::search(const std::string& prefix) const {
  std::vector<std::string> results;
  if (prefix.empty()) {
    return results;
  }
  searchHelper(root_.get(), prefix, results, 0);
  return results;
}

void RadixTreeIndex::searchHelper(const RadixNode* node,
                                   const std::string& prefix,
                                   std::vector<std::string>& results,
                                   size_t depth) const {
  // If we've matched the entire prefix, collect all IDs from this subtree
  if (depth >= prefix.length()) {
    collectAllIds(node, results);
    return;
  }

  std::string remaining = prefix.substr(depth);

  // Search through children
  for (const auto& child : node->children) {
    const std::string& edge_label = child->edge_label;

    // Check if the edge_label matches the remaining prefix
    if (remaining.length() <= edge_label.length()) {
      // Check if remaining is a prefix of edge_label
      if (edge_label.substr(0, remaining.length()) == remaining) {
        // We've matched the entire prefix, collect all IDs from this subtree
        collectAllIds(child.get(), results);
        return;
      }
    } else {
      // Check if edge_label is a prefix of remaining
      if (remaining.substr(0, edge_label.length()) == edge_label) {
        // Continue searching down this path
        searchHelper(child.get(), prefix, results, depth + edge_label.length());
        return;
      }
    }
  }
}

void RadixTreeIndex::collectAllIds(const RadixNode* node,
                                    std::vector<std::string>& results) const {
  // Add all address_ids from this node
  for (const auto& id : node->address_ids) {
    // Avoid duplicates
    if (std::find(results.begin(), results.end(), id) == results.end()) {
      results.push_back(id);
    }
  }

  // Recursively collect from all children (in sorted order for determinism)
  for (const auto& child : node->children) {
    collectAllIds(child.get(), results);
  }
}

size_t RadixTreeIndex::getMemoryUsage() const {
  return getMemoryUsageHelper(root_.get());
}

size_t RadixTreeIndex::getMemoryUsageHelper(const RadixNode* node) const {
  if (!node) {
    return 0;
  }

  size_t usage = sizeof(RadixNode);
  usage += node->edge_label.capacity();
  usage += node->address_ids.capacity() * sizeof(std::string);
  for (const auto& id : node->address_ids) {
    usage += id.capacity();
  }
  usage += node->children.capacity() * sizeof(std::unique_ptr<RadixNode>);

  for (const auto& child : node->children) {
    usage += getMemoryUsageHelper(child.get());
  }

  return usage;
}

size_t RadixTreeIndex::getTermCount() const {
  return term_count_;
}
