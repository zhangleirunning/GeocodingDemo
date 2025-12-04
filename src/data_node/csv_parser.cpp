#include "data_node/csv_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>

CSVParser::CSVParser() : success_count_(0), error_count_(0) {}

std::vector<AddressRecord> CSVParser::parse(const std::string& filepath) {
  std::vector<AddressRecord> records;
  std::ifstream file(filepath);

  if (!file.is_open()) {
    std::cerr << "Error: Could not open CSV file: " << filepath << std::endl;
    return records;
  }

  std::string line;
  bool is_header = true;

  // Reset counters for new parse operation
  success_count_ = 0;
  error_count_ = 0;

  while (std::getline(file, line)) {
    // Skip header line
    if (is_header) {
      is_header = false;
      continue;
    }

    // Skip empty lines
    if (trim(line).empty()) {
      continue;
    }

    auto record = parseRecord(line);
    if (record.has_value()) {
      records.push_back(record.value());
      success_count_++;
    } else {
      error_count_++;
    }
  }

  file.close();
  return records;
}

size_t CSVParser::getSuccessCount() const { return success_count_; }

size_t CSVParser::getErrorCount() const { return error_count_; }

std::optional<AddressRecord> CSVParser::parseRecord(const std::string& line) {
  std::vector<std::string> fields = splitCSVLine(line);

  // CSV format: LON,LAT,NUMBER,STREET,UNIT,CITY,DISTRICT,REGION,POSTCODE,ID,HASH
  // We need at least 11 fields
  if (fields.size() < 11) {
    std::cerr << "Warning: Malformed record - insufficient fields (expected 11, got "
              << fields.size() << ")" << std::endl;
    return std::nullopt;
  }

  try {
    // Parse coordinates
    double longitude = std::stod(fields[0]);
    double latitude = std::stod(fields[1]);

    // Validate coordinates
    if (!validateCoordinates(longitude, latitude)) {
      std::cerr << "Warning: Invalid coordinates - lon=" << longitude
                << ", lat=" << latitude << std::endl;
      return std::nullopt;
    }

    // Extract other fields (handle empty fields gracefully)
    std::string number = fields[2];
    std::string street = fields[3];
    std::string unit = fields[4];
    std::string city = fields[5];
    // Note: DISTRICT (fields[6]) and REGION (fields[7]) are not stored in AddressRecord
    std::string postcode = fields[8];
    // Note: ID (fields[9]) is not stored in AddressRecord
    std::string hash_str = fields[10];

    // Convert hex string to size_t
    size_t hash = 0;
    if (!hash_str.empty()) {
      hash = std::stoull(hash_str, nullptr, 16);
    }

    // Create AddressRecord with original values
    // Note: normalized values will be set later by the normalizer
    AddressRecord record(longitude, latitude, hash, number, street, unit, city,
                         postcode, street, unit, city);

    return record;

  } catch (const std::invalid_argument& e) {
    std::cerr << "Warning: Invalid number format in record" << std::endl;
    return std::nullopt;
  } catch (const std::out_of_range& e) {
    std::cerr << "Warning: Number out of range in record" << std::endl;
    return std::nullopt;
  }
}

bool CSVParser::validateCoordinates(double lon, double lat) const {
  // Longitude must be in range [-180, 180]
  // Latitude must be in range [-90, 90]
  return (lon >= -180.0 && lon <= 180.0) && (lat >= -90.0 && lat <= 90.0);
}

std::vector<std::string> CSVParser::splitCSVLine(const std::string& line) const {
  std::vector<std::string> fields;
  std::string field;
  bool in_quotes = false;

  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];

    if (c == '"') {
      in_quotes = !in_quotes;
    } else if (c == ',' && !in_quotes) {
      fields.push_back(field);
      field.clear();
    } else {
      field += c;
    }
  }

  // Add the last field
  fields.push_back(field);

  return fields;
}

std::string CSVParser::trim(const std::string& str) const {
  size_t start = 0;
  size_t end = str.length();

  // Find first non-whitespace character
  while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
    ++start;
  }

  // Find last non-whitespace character
  while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
    --end;
  }

  return str.substr(start, end - start);
}
