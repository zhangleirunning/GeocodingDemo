// Address Normalizer Unit Tests
// Requirements: 2.1, 2.2, 2.3, 2.4

#include "data_node/address_normalizer.h"

#include <gtest/gtest.h>

// Test case conversion with mixed-case inputs (Requirement 2.1)
TEST(AddressNormalizerTest, CaseConversion) {
  AddressNormalizer normalizer;

  // Test mixed case
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main Street"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("main street"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("MAIN STREET"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("MaIn StReEt"));

  // Test with numbers
  EXPECT_EQ("123 MAIN ST", normalizer.normalize("123 Main St"));

  // Test empty string
  EXPECT_EQ("", normalizer.normalize(""));
}

// Test whitespace handling with known inputs (Requirements 2.2, 2.3)
TEST(AddressNormalizerTest, WhitespaceHandling) {
  AddressNormalizer normalizer;

  // Test leading whitespace removal (Requirement 2.2)
  EXPECT_EQ("MAIN STREET", normalizer.normalize("  Main Street"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("   Main Street"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("\tMain Street"));

  // Test trailing whitespace removal (Requirement 2.2)
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main Street  "));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main Street   "));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main Street\t"));

  // Test both leading and trailing whitespace
  EXPECT_EQ("MAIN STREET", normalizer.normalize("  Main Street  "));

  // Test multiple consecutive spaces collapsed to single space (Requirement 2.3)
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main  Street"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main   Street"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main    Street"));

  // Test multiple spaces in multiple locations
  EXPECT_EQ("123 MAIN STREET", normalizer.normalize("123  Main   Street"));

  // Test tabs and other whitespace characters
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main\t\tStreet"));
  EXPECT_EQ("MAIN STREET", normalizer.normalize("Main\n\nStreet"));

  // Test combination of leading, trailing, and multiple spaces
  EXPECT_EQ("MAIN STREET", normalizer.normalize("  Main   Street  "));
}

// Test specific street suffix abbreviations (Requirement 2.4)
TEST(AddressNormalizerTest, StreetSuffixAbbreviations) {
  AddressNormalizer normalizer;

  // Test ST -> STREET
  EXPECT_EQ("MAIN STREET", normalizer.normalizeStreetSuffix("Main St"));
  EXPECT_EQ("3RD STREET", normalizer.normalizeStreetSuffix("3rd St"));

  // Test AVE -> AVENUE
  EXPECT_EQ("PARK AVENUE", normalizer.normalizeStreetSuffix("Park Ave"));
  EXPECT_EQ("5TH AVENUE", normalizer.normalizeStreetSuffix("5th Ave"));

  // Test RD -> ROAD
  EXPECT_EQ("MAPLE ROAD", normalizer.normalizeStreetSuffix("Maple Rd"));
  EXPECT_EQ("OAK ROAD", normalizer.normalizeStreetSuffix("Oak Rd"));

  // Test BLVD -> BOULEVARD
  EXPECT_EQ("SUNSET BOULEVARD", normalizer.normalizeStreetSuffix("Sunset Blvd"));

  // Test DR -> DRIVE
  EXPECT_EQ("OCEAN DRIVE", normalizer.normalizeStreetSuffix("Ocean Dr"));

  // Test LN -> LANE
  EXPECT_EQ("CHERRY LANE", normalizer.normalizeStreetSuffix("Cherry Ln"));

  // Test CT -> COURT
  EXPECT_EQ("WILLOW COURT", normalizer.normalizeStreetSuffix("Willow Ct"));

  // Test PL -> PLACE
  EXPECT_EQ("PINE PLACE", normalizer.normalizeStreetSuffix("Pine Pl"));
}

// Test street suffix with mixed case and whitespace
TEST(AddressNormalizerTest, StreetSuffixWithWhitespace) {
  AddressNormalizer normalizer;

  // Test with leading/trailing whitespace
  EXPECT_EQ("MAIN STREET", normalizer.normalizeStreetSuffix("  Main St  "));

  // Test with multiple spaces
  EXPECT_EQ("MAIN STREET", normalizer.normalizeStreetSuffix("Main   St"));

  // Test with mixed case
  EXPECT_EQ("MAIN STREET", normalizer.normalizeStreetSuffix("main st"));
  EXPECT_EQ("MAIN STREET", normalizer.normalizeStreetSuffix("MAIN ST"));
}

// Test street names without abbreviations
TEST(AddressNormalizerTest, StreetSuffixNoAbbreviation) {
  AddressNormalizer normalizer;

  // Test street names that are already expanded
  EXPECT_EQ("MAIN STREET", normalizer.normalizeStreetSuffix("Main Street"));
  EXPECT_EQ("PARK AVENUE", normalizer.normalizeStreetSuffix("Park Avenue"));

  // Test street names without suffix
  EXPECT_EQ("BROADWAY", normalizer.normalizeStreetSuffix("Broadway"));

  // Test street names with unknown suffix
  EXPECT_EQ("MAIN ALLEY", normalizer.normalizeStreetSuffix("Main Alley"));
}

// Test edge cases
TEST(AddressNormalizerTest, EdgeCases) {
  AddressNormalizer normalizer;

  // Test empty string
  EXPECT_EQ("", normalizer.normalize(""));
  EXPECT_EQ("", normalizer.normalizeStreetSuffix(""));

  // Test string with only whitespace
  EXPECT_EQ("", normalizer.normalize("   "));
  EXPECT_EQ("", normalizer.normalizeStreetSuffix("   "));

  // Test single word
  EXPECT_EQ("STREET", normalizer.normalize("street"));

  // Test single character
  EXPECT_EQ("A", normalizer.normalize("a"));
}

// Test normalization preserves numeric values (Requirement 2.5)
TEST(AddressNormalizerTest, NumericPreservation) {
  AddressNormalizer normalizer;

  // Test street numbers are preserved
  EXPECT_EQ("123 MAIN STREET", normalizer.normalize("123 Main Street"));
  EXPECT_EQ("456", normalizer.normalize("456"));

  // Test apartment/unit numbers
  EXPECT_EQ("APT 5B", normalizer.normalize("Apt 5B"));
  EXPECT_EQ("UNIT 42", normalizer.normalize("Unit 42"));

  // Test postcodes
  EXPECT_EQ("98388", normalizer.normalize("98388"));
  EXPECT_EQ("12345-6789", normalizer.normalize("12345-6789"));
}
