#ifndef DATA_NODE_CSV_PARSER_H_
#define DATA_NODE_CSV_PARSER_H_

#include <optional>
#include <string>
#include <vector>

#include "data_node/address_record.h"

class CSVParser {
 public:
  CSVParser();

  // Parse CSV file and return vector of address records
  std::vector<AddressRecord> parse(const std::string& filepath);

  // Get count of successfully parsed records
  size_t getSuccessCount() const;

  // Get count of failed/malformed records
  size_t getErrorCount() const;

 private:
  size_t success_count_;
  size_t error_count_;

  // Parse a single CSV record line
  std::optional<AddressRecord> parseRecord(const std::string& line);

  // Validate coordinate ranges
  bool validateCoordinates(double lon, double lat) const;

  // Helper to split CSV line into fields
  std::vector<std::string> splitCSVLine(const std::string& line) const;

  // Helper to trim whitespace from string
  std::string trim(const std::string& str) const;
};

#endif  // DATA_NODE_CSV_PARSER_H_
