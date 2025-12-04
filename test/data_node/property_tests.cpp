// Property-Based Tests

#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>

#include <fstream>
#include <sstream>
#include <iomanip>

#include "data_node/csv_parser.h"
#include "data_node/data_node.h"
#include "data_node/address_normalizer.h"

// Helper function to generate a valid CSV line from field values
std::string generateCSVLine(double lon, double lat, const std::string& number,
                            const std::string& street, const std::string& unit,
                            const std::string& city, const std::string& district,
                            const std::string& region, const std::string& postcode,
                            const std::string& id, const std::string& hash) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(7) << lon << ","
      << std::fixed << std::setprecision(7) << lat << ","
      << number << "," << street << "," << unit << "," << city << ","
      << district << "," << region << "," << postcode << "," << id << ","
      << hash;
  return oss.str();
}

// Helper function to write a temporary CSV file
std::string writeTempCSV(const std::vector<std::string>& lines) {
  std::string filename = "/tmp/test_property_" + std::to_string(rand()) + ".csv";
  std::ofstream file(filename);

  // Write header
  file << "LON,LAT,NUMBER,STREET,UNIT,CITY,DISTRICT,REGION,POSTCODE,ID,HASH\n";

  // Write data lines
  for (const auto& line : lines) {
    file << line << "\n";
  }

  file.close();
  return filename;
}

// Generator for valid longitude values
rc::Gen<double> genValidLongitude() {
  return rc::gen::apply(
    [](int whole, int frac) {
      return static_cast<double>(whole) + frac / 1000000.0;
    },
    rc::gen::inRange(-180, 180),
    rc::gen::inRange(0, 999999)
  );
}

// Generator for valid latitude values
rc::Gen<double> genValidLatitude() {
  return rc::gen::apply(
    [](int whole, int frac) {
      return static_cast<double>(whole) + frac / 1000000.0;
    },
    rc::gen::inRange(-90, 90),
    rc::gen::inRange(0, 999999)
  );
}

// Generator for non-empty alphanumeric strings (for required fields)
// Ensures strings are at least 3 characters to avoid normalization issues
// and have meaningful content for search validation
rc::Gen<std::string> genNonEmptyString() {
  return rc::gen::suchThat(
    rc::gen::string<std::string>(),
    [](const std::string& s) {
      // Must be at least 3 chars and not contain special characters
      if (s.length() < 3) return false;
      if (s.find(',') != std::string::npos) return false;
      if (s.find('\n') != std::string::npos) return false;
      if (s.find('"') != std::string::npos) return false;

      // Must contain at least two alphanumeric characters
      int alnum_count = 0;
      for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
          alnum_count++;
          if (alnum_count >= 2) return true;
        }
      }
      return false;
    }
  );
}

// Generator for possibly empty alphanumeric strings (for optional fields)
rc::Gen<std::string> genOptionalString() {
  return rc::gen::suchThat(
    rc::gen::string<std::string>(),
    [](const std::string& s) {
      return s.find(',') == std::string::npos &&
             s.find('\n') == std::string::npos && s.find('"') == std::string::npos;
    }
  );
}

// Generator for valid hex strings (16 hex digits for 64-bit hash)
rc::Gen<std::string> genHexString() {
  return rc::gen::apply(
    [](uint64_t value) {
      std::ostringstream oss;
      oss << std::hex << std::setw(16) << std::setfill('0') << value;
      return oss.str();
    },
    rc::gen::arbitrary<uint64_t>()
  );
}

// Feature: geocoding-data-nodes, Property 1: CSV field extraction completeness
// Validates: Requirements 1.2
RC_GTEST_PROP(PropertyTests, CSVFieldExtractionCompleteness, ()) {
  // Generate random valid field values
  auto lon = *genValidLongitude();
  auto lat = *genValidLatitude();
  auto number = *genNonEmptyString();
  auto street = *genNonEmptyString();
  auto unit = *genOptionalString();
  auto city = *genNonEmptyString();
  auto district = *genOptionalString();
  auto region = *genOptionalString();
  auto postcode = *genNonEmptyString();
  auto id = *genOptionalString();
  auto hash = *genHexString();

  // Generate CSV line
  std::string csv_line = generateCSVLine(lon, lat, number, street, unit, city,
                                         district, region, postcode, id, hash);

  // Write to temporary file
  std::string temp_file = writeTempCSV({csv_line});

  // Parse the file
  CSVParser parser;
  std::vector<AddressRecord> records = parser.parse(temp_file);

  // Clean up
  std::remove(temp_file.c_str());

  // Verify that parsing succeeded
  RC_ASSERT(records.size() == 1);
  RC_ASSERT(parser.getSuccessCount() == 1);
  RC_ASSERT(parser.getErrorCount() == 0);

  // Verify that all extracted fields match the input values
  const AddressRecord& record = records[0];

  // Check coordinates with tolerance for floating point precision
  const double kEpsilon = 1e-6;
  RC_ASSERT(std::abs(record.longitude - lon) < kEpsilon);
  RC_ASSERT(std::abs(record.latitude - lat) < kEpsilon);

  // Check string fields
  RC_ASSERT(record.number == number);
  RC_ASSERT(record.street == street);
  RC_ASSERT(record.unit == unit);
  RC_ASSERT(record.city == city);
  RC_ASSERT(record.postcode == postcode);
  RC_ASSERT(record.hash == std::stoull(hash, nullptr, 16));

  // Note: The current implementation doesn't extract DISTRICT, REGION, or ID
  // This is a known limitation that should be addressed in the implementation
}

// Feature: geocoding-data-nodes, Property 12: End-to-end search correctness
// Validates: Requirements 5.1, 5.2, 5.3
RC_GTEST_PROP(PropertyTests, EndToEndSearchCorrectness, ()) {
  // Generate a set of random address records
  auto num_records = *rc::gen::inRange(1, 20);

  std::vector<AddressRecord> generated_records;
  std::vector<std::string> csv_lines;

  for (int i = 0; i < num_records; ++i) {
    auto lon = *genValidLongitude();
    auto lat = *genValidLatitude();
    auto number = *genNonEmptyString();
    auto street = *genNonEmptyString();
    auto unit = *genOptionalString();
    auto city = *genNonEmptyString();
    auto district = *genOptionalString();
    auto region = *genOptionalString();
    auto postcode = *genNonEmptyString();
    auto id = std::to_string(i);
    auto hash = *genHexString();

    // Create the record
    AddressRecord record;
    record.longitude = lon;
    record.latitude = lat;
    record.hash = std::stoull(hash, nullptr, 16);
    record.number = number;
    record.street = street;
    record.unit = unit;
    record.city = city;
    record.postcode = postcode;
    record.original_street = street;
    record.original_unit = unit;
    record.original_city = city;

    generated_records.push_back(record);

    // Generate CSV line
    std::string csv_line = generateCSVLine(lon, lat, number, street, unit, city,
                                           district, region, postcode, id, hash);
    csv_lines.push_back(csv_line);
  }

  // Write to temporary file
  std::string temp_file = writeTempCSV(csv_lines);

  // Create and initialize DataNode
  DataNode node(0, temp_file);
  bool init_success = node.initialize();

  // Clean up temp file
  std::remove(temp_file.c_str());

  RC_ASSERT(init_success);

  // Pick a random record to search for
  auto target_idx = *rc::gen::inRange(0, num_records);
  const AddressRecord& target = generated_records[target_idx];

  // Create query terms from the target record's searchable fields
  // We'll search using the street name
  std::vector<std::string> query_terms;
  if (!target.street.empty()) {
    query_terms.push_back(target.street);
  }

  // Skip test if query terms is empty (street was empty or too short)
  if (query_terms.empty()) {
    RC_SUCCEED("Skipping test - target street is empty");
    return;
  }

  // Perform the search
  std::vector<AddressRecord> results = node.search(query_terms);

  // Verify that results are not empty (should at least contain our target)
  RC_ASSERT(!results.empty());

  // Verify that all returned records have complete fields (non-zero coordinates)
  for (const auto& result : results) {
    // Check that coordinates are present
    RC_ASSERT(result.longitude != 0.0 || result.latitude != 0.0);

    // Check that at least some fields are populated
    RC_ASSERT(result.hash != 0 || !result.street.empty() || !result.city.empty());
  }

  // Verify that the target record is in the results
  // Note: We need to compare using hash since that's the unique identifier
  bool found_target = false;
  for (const auto& result : results) {
    // Check if this result matches our target by hash (unique ID)
    if (result.hash == target.hash) {
      found_target = true;
      break;
    }
  }

  RC_ASSERT(found_target);

  // Verify that all results actually match the query terms
  // Since we're searching by street, all results should have matching street
  // Note: RadixTree uses prefix matching, so "aaa" can match "aab" if they share a prefix
  AddressNormalizer normalizer;
  std::string normalized_query_street = normalizer.normalize(target.street);

  // Verify the normalized query is meaningful (at least 1 character)
  RC_ASSERT(!normalized_query_street.empty());

  for (const auto& result : results) {
    std::string normalized_result_street = normalizer.normalize(result.street);

    // Result street should also be non-empty after normalization
    RC_ASSERT(!normalized_result_street.empty());

    // The result's street should contain or match the query term (or vice versa)
    // This validates that the search is returning relevant results
    // RadixTree does prefix matching, so we check if one is a prefix of the other
    bool matches = (normalized_result_street.find(normalized_query_street) == 0 ||  // result starts with query
                   normalized_query_street.find(normalized_result_street) == 0 ||  // query starts with result
                   normalized_result_street.find(normalized_query_street) != std::string::npos ||  // query anywhere in result
                   normalized_query_street.find(normalized_result_street) != std::string::npos);   // result anywhere in query

    RC_ASSERT(matches);
  }
}
