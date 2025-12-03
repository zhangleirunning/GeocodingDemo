#include "data_node/address_normalizer.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

AddressNormalizer::AddressNormalizer() {
  // Initialize suffix map with common abbreviations
  suffix_map_["ST"] = "STREET";
  suffix_map_["AVE"] = "AVENUE";
  suffix_map_["RD"] = "ROAD";
  suffix_map_["BLVD"] = "BOULEVARD";
  suffix_map_["DR"] = "DRIVE";
  suffix_map_["LN"] = "LANE";
  suffix_map_["CT"] = "COURT";
  suffix_map_["PL"] = "PLACE";
  suffix_map_["CIR"] = "CIRCLE";
  suffix_map_["WAY"] = "WAY";
  suffix_map_["PKWY"] = "PARKWAY";
  suffix_map_["TER"] = "TERRACE";
  suffix_map_["SQ"] = "SQUARE";
  suffix_map_["HWY"] = "HIGHWAY";
  suffix_map_["EXPY"] = "EXPRESSWAY";
}

std::string AddressNormalizer::normalize(const std::string& text) {
  // Apply normalization steps in order:
  // 1. Convert to uppercase
  // 2. Trim leading/trailing whitespace
  // 3. Collapse multiple spaces to single space
  std::string result = toUpperCase(text);
  result = trimWhitespace(result);
  result = collapseWhitespace(result);
  return result;
}

std::string AddressNormalizer::normalizeStreetSuffix(const std::string& street) {
  // First apply general normalization
  std::string normalized = normalize(street);

  // Split the street into words
  std::istringstream iss(normalized);
  std::vector<std::string> words;
  std::string word;

  while (iss >> word) {
    words.push_back(word);
  }

  // If there are no words, return empty string
  if (words.empty()) {
    return normalized;
  }

  // Check if the last word is a known abbreviation
  std::string& last_word = words.back();
  auto it = suffix_map_.find(last_word);
  if (it != suffix_map_.end()) {
    last_word = it->second;
  }

  // Reconstruct the street name
  std::ostringstream oss;
  for (size_t i = 0; i < words.size(); ++i) {
    if (i > 0) {
      oss << " ";
    }
    oss << words[i];
  }

  return oss.str();
}

std::string AddressNormalizer::toUpperCase(const std::string& text) {
  std::string result = text;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return result;
}

std::string AddressNormalizer::trimWhitespace(const std::string& text) {
  if (text.empty()) {
    return text;
  }

  size_t start = 0;
  size_t end = text.length();

  // Find first non-whitespace character
  while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) {
    ++start;
  }

  // Find last non-whitespace character
  while (end > start &&
         std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    --end;
  }

  return text.substr(start, end - start);
}

std::string AddressNormalizer::collapseWhitespace(const std::string& text) {
  if (text.empty()) {
    return text;
  }

  std::string result;
  result.reserve(text.length());

  bool prev_was_space = false;

  for (char c : text) {
    bool is_space = std::isspace(static_cast<unsigned char>(c));

    if (is_space) {
      if (!prev_was_space) {
        result += ' ';
        prev_was_space = true;
      }
    } else {
      result += c;
      prev_was_space = false;
    }
  }

  return result;
}
