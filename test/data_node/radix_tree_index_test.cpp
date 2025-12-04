// Radix Tree Index Unit Tests

#include "data_node/radix_tree_index.h"

#include <gtest/gtest.h>

// Basic functionality test to verify the implementation works
TEST(RadixTreeIndexTest, BasicInsertAndSearch) {
  RadixTreeIndex index;

  // Insert some terms
  index.insert("STREET", 1);
  index.insert("STREET", 2);
  index.insert("STREAM", 3);
  index.insert("STRONG", 4);

  // Search for exact match
  auto results = index.search("STREET");
  EXPECT_EQ(results.size(), 2);

  // Search for prefix
  results = index.search("STR");
  EXPECT_EQ(results.size(), 4);  // Should match STREET (2 ids), STREAM, STRONG

  // Search for non-existent prefix
  results = index.search("XYZ");
  EXPECT_EQ(results.size(), 0);
}

TEST(RadixTreeIndexTest, EmptySearchReturnsEmpty) {
  RadixTreeIndex index;
  index.insert("TEST", 1);

  auto results = index.search("");
  EXPECT_EQ(results.size(), 0);
}

TEST(RadixTreeIndexTest, EmptyInsertIgnored) {
  RadixTreeIndex index;
  index.insert("", 1);

  EXPECT_EQ(index.getTermCount(), 0);
}

TEST(RadixTreeIndexTest, GetMemoryUsage) {
  RadixTreeIndex index;
  size_t initial_usage = index.getMemoryUsage();

  index.insert("TEST", 1);
  size_t after_insert = index.getMemoryUsage();

  EXPECT_GT(after_insert, initial_usage);
}

TEST(RadixTreeIndexTest, GetTermCount) {
  RadixTreeIndex index;
  EXPECT_EQ(index.getTermCount(), 0);

  index.insert("TERM1", 1);
  EXPECT_EQ(index.getTermCount(), 1);

  index.insert("TERM2", 2);
  EXPECT_EQ(index.getTermCount(), 2);

  // Inserting same term with different ID should still increment count
  index.insert("TERM1", 3);
  EXPECT_EQ(index.getTermCount(), 3);
}

// Task 5.2: Test inserting and searching single term
// Requirements: 3.2
TEST(RadixTreeIndexTest, InsertAndSearchSingleTerm) {
  RadixTreeIndex index;

  // Insert a single term with one ID
  index.insert("MAIN", 0x1234567890ABCDEF);

  // Search for the exact term
  auto results = index.search("MAIN");
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x1234567890ABCDEF);

  // Search for a prefix of the term
  results = index.search("MA");
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x1234567890ABCDEF);

  // Search for the full term as prefix
  results = index.search("MAIN");
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x1234567890ABCDEF);

  // Search for non-matching prefix
  results = index.search("SIDE");
  EXPECT_EQ(results.size(), 0);
}

// Task 5.2: Test prefix search with multiple matching terms
// Requirements: 3.4
TEST(RadixTreeIndexTest, PrefixSearchMultipleMatches) {
  RadixTreeIndex index;

  // Insert multiple terms that share a common prefix
  index.insert("MAIN", 0x1111111111111111);
  index.insert("MAPLE", 0x2222222222222222);
  index.insert("MARKET", 0x3333333333333333);
  index.insert("MADISON", 0x4444444444444444);
  index.insert("BROAD", 0x5555555555555555);  // Different prefix

  // Search for common prefix "MA" - should match MAIN, MAPLE, MARKET, MADISON
  auto results = index.search("MA");
  EXPECT_EQ(results.size(), 4);
  // Results should contain all four IDs
  EXPECT_NE(std::find(results.begin(), results.end(), 0x1111111111111111), results.end());
  EXPECT_NE(std::find(results.begin(), results.end(), 0x2222222222222222), results.end());
  EXPECT_NE(std::find(results.begin(), results.end(), 0x3333333333333333), results.end());
  EXPECT_NE(std::find(results.begin(), results.end(), 0x4444444444444444), results.end());
  // Should not contain BROAD
  EXPECT_EQ(std::find(results.begin(), results.end(), 0x5555555555555555), results.end());

  // Search for more specific prefix "MAR" - should match MARKET only
  results = index.search("MAR");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x3333333333333333);

  // Search for prefix "B" - should match BROAD only
  results = index.search("B");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x5555555555555555);

  // Search for prefix "M" - should match all MA* terms
  results = index.search("M");
  EXPECT_EQ(results.size(), 4);
}

// Task 5.2: Test duplicate term handling
// Requirements: 3.3
TEST(RadixTreeIndexTest, DuplicateTermHandling) {
  RadixTreeIndex index;

  // Insert the same term with multiple different IDs
  index.insert("STREET", 0xAAAAAAAAAAAAAAAA);
  index.insert("STREET", 0xBBBBBBBBBBBBBBBB);
  index.insert("STREET", 0xCCCCCCCCCCCCCCCC);

  // Search should return all IDs associated with the term
  auto results = index.search("STREET");
  EXPECT_EQ(results.size(), 3);
  EXPECT_NE(std::find(results.begin(), results.end(), 0xAAAAAAAAAAAAAAAA), results.end());
  EXPECT_NE(std::find(results.begin(), results.end(), 0xBBBBBBBBBBBBBBBB), results.end());
  EXPECT_NE(std::find(results.begin(), results.end(), 0xCCCCCCCCCCCCCCCC), results.end());

  // Prefix search should also return all IDs
  results = index.search("STR");
  EXPECT_EQ(results.size(), 3);

  // Insert the same term-ID pair again (duplicate)
  index.insert("STREET", 0xAAAAAAAAAAAAAAAA);

  // Should still have 3 unique IDs (no duplicate IDs)
  results = index.search("STREET");
  EXPECT_EQ(results.size(), 3);

  // Add another unique ID
  index.insert("STREET", 0xDDDDDDDDDDDDDDDD);
  results = index.search("STREET");
  EXPECT_EQ(results.size(), 4);
}

// Task 5.2: Additional test for duplicate terms with overlapping prefixes
// Requirements: 3.3, 3.4
TEST(RadixTreeIndexTest, DuplicateTermsWithPrefixSearch) {
  RadixTreeIndex index;

  // Insert multiple terms, some duplicates
  index.insert("PARK", 0x0000000000000001);
  index.insert("PARK", 0x0000000000000002);
  index.insert("PARKER", 0x0000000000000003);
  index.insert("PARKING", 0x0000000000000004);
  index.insert("PARK", 0x0000000000000005);

  // Search for exact term "PARK" - should return 3 IDs
  auto results = index.search("PARK");
  EXPECT_EQ(results.size(), 5);  // All terms starting with PARK

  // Search for "PARKER" - should return 1 ID
  results = index.search("PARKER");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x0000000000000003);

  // Search for "PARKING" - should return 1 ID
  results = index.search("PARKING");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 0x0000000000000004);

  // Search for prefix "PAR" - should return all 5 IDs
  results = index.search("PAR");
  EXPECT_EQ(results.size(), 5);
}
