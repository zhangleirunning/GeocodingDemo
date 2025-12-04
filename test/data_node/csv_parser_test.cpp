// CSV Parser Unit Tests
// Requirements: 1.1, 1.2, 1.3, 1.4

#include <gtest/gtest.h>

#include <fstream>

#include "data_node/csv_parser.h"

// Test parsing well-formed CSV with all fields
TEST(CSVParserTest, ParseWellFormedCSV) {
  CSVParser parser;
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/valid_addresses.csv");

  // Should successfully parse all 5 records
  EXPECT_EQ(records.size(), 5);
  EXPECT_EQ(parser.getSuccessCount(), 5);
  EXPECT_EQ(parser.getErrorCount(), 0);

  // Verify first record fields
  EXPECT_DOUBLE_EQ(records[0].longitude, -122.608996);
  EXPECT_DOUBLE_EQ(records[0].latitude, 47.166377);
  EXPECT_EQ(records[0].number, "611");
  EXPECT_EQ(records[0].street, "3RD ST");
  EXPECT_EQ(records[0].unit, "");
  EXPECT_EQ(records[0].city, "Steilacoom");
  EXPECT_EQ(records[0].postcode, "98388");
  EXPECT_EQ(records[0].hash, 0x46a6ea62641c0d1c);

  // Verify second record with unit field
  EXPECT_DOUBLE_EQ(records[1].longitude, -121.6461331);
  EXPECT_DOUBLE_EQ(records[1].latitude, 36.7082169);
  EXPECT_EQ(records[1].number, "1531");
  EXPECT_EQ(records[1].street, "MCKINNON STREET");
  EXPECT_EQ(records[1].unit, "C");
  EXPECT_EQ(records[1].city, "Salinas");
  EXPECT_EQ(records[1].postcode, "93906");
  EXPECT_EQ(records[1].hash, 0xa8ac1dc8c998ce76);
}

// Test parsing CSV with empty optional fields
TEST(CSVParserTest, ParseCSVWithEmptyOptionalFields) {
  CSVParser parser;
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/empty_optional_fields.csv");

  // Should successfully parse all 3 records despite empty fields
  EXPECT_EQ(records.size(), 3);
  EXPECT_EQ(parser.getSuccessCount(), 3);
  EXPECT_EQ(parser.getErrorCount(), 0);

  // Verify record with empty unit field
  EXPECT_EQ(records[1].unit, "");
  EXPECT_EQ(records[1].street, "MCKINNON STREET");

  // Verify record with empty number field
  EXPECT_EQ(records[2].number, "");
  EXPECT_EQ(records[2].street, "LEYTE ROAD");
}

// Test handling missing CSV file
TEST(CSVParserTest, HandleMissingCSVFile) {
  CSVParser parser;
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/nonexistent_file.csv");

  // Should return empty vector when file doesn't exist
  EXPECT_EQ(records.size(), 0);
  EXPECT_EQ(parser.getSuccessCount(), 0);
  EXPECT_EQ(parser.getErrorCount(), 0);
}

// Test specific malformed records
TEST(CSVParserTest, HandleMalformedRecords) {
  CSVParser parser;
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/malformed_addresses.csv");

  // Should parse only valid records and skip malformed ones
  // Valid: record 1 (index 0) and record 7 (index 6)
  // Invalid: records with invalid_lon, invalid_lat, out-of-range coords,
  // insufficient fields
  EXPECT_EQ(records.size(), 2);
  EXPECT_EQ(parser.getSuccessCount(), 2);
  EXPECT_EQ(parser.getErrorCount(), 5);

  // Verify the valid records were parsed correctly
  EXPECT_DOUBLE_EQ(records[0].longitude, -122.608996);
  EXPECT_DOUBLE_EQ(records[0].latitude, 47.166377);
  EXPECT_EQ(records[0].number, "611");

  EXPECT_DOUBLE_EQ(records[1].longitude, -121.6328919);
  EXPECT_DOUBLE_EQ(records[1].latitude, 36.6760439);
  EXPECT_EQ(records[1].number, "68");
}

// Test coordinate validation
TEST(CSVParserTest, ValidateCoordinateRanges) {
  CSVParser parser;
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/malformed_addresses.csv");

  // Records with lon=-200.0 and lat=100.0 should be rejected
  for (const auto& record : records) {
    EXPECT_GE(record.longitude, -180.0);
    EXPECT_LE(record.longitude, 180.0);
    EXPECT_GE(record.latitude, -90.0);
    EXPECT_LE(record.latitude, 90.0);
  }
}

// Test that parser handles records with insufficient fields
TEST(CSVParserTest, HandleInsufficientFields) {
  CSVParser parser;
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/malformed_addresses.csv");

  // The record with only 7 fields should be rejected (needs 11)
  // Verify by checking error count includes this case
  EXPECT_GT(parser.getErrorCount(), 0);
}

// Test that parser resets counters on new parse
TEST(CSVParserTest, ResetCountersOnNewParse) {
  CSVParser parser;

  // First parse
  parser.parse("test/fixtures/valid_addresses.csv");
  EXPECT_EQ(parser.getSuccessCount(), 5);
  EXPECT_EQ(parser.getErrorCount(), 0);

  // Second parse should reset counters
  std::vector<AddressRecord> records =
      parser.parse("test/fixtures/malformed_addresses.csv");
  EXPECT_EQ(parser.getSuccessCount(), 2);
  EXPECT_EQ(parser.getErrorCount(), 5);
}
