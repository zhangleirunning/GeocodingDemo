#ifndef DATA_NODE_ADDRESS_NORMALIZER_H_
#define DATA_NODE_ADDRESS_NORMALIZER_H_

#include <string>
#include <unordered_map>

class AddressNormalizer {
 public:
  AddressNormalizer();

  // Normalize a single address field
  std::string normalize(const std::string& text);

  // Normalize street suffix abbreviations
  std::string normalizeStreetSuffix(const std::string& street);

 private:
  std::string toUpperCase(const std::string& text);
  std::string trimWhitespace(const std::string& text);
  std::string collapseWhitespace(const std::string& text);

  // Map of common abbreviations to standard forms
  std::unordered_map<std::string, std::string> suffix_map_;
};

#endif  // DATA_NODE_ADDRESS_NORMALIZER_H_
