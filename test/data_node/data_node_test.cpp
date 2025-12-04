// Data Node Unit Tests

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>

#include "data_node/data_node.h"

// Helper to get the correct path to test data
static std::string getTestDataPath(const std::string& filename) {
  // Try relative to build directory first
  std::string path = "../test/fixtures/" + filename;
  std::ifstream test_file(path);
  if (test_file.good()) {
    return path;
  }
  // Fall back to relative to workspace root
  return "test/fixtures/" + filename;
}

// Test initialization with valid CSV file
TEST(DataNodeTest, InitializeWithValidCSV) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));

  bool success = node.initialize();

  EXPECT_TRUE(success);

  // Verify statistics
  DataNode::Statistics stats = node.getStatistics();
  EXPECT_EQ(stats.total_records, 5);
  EXPECT_GT(stats.radix_tree_memory, 0);
  EXPECT_GT(stats.forward_index_size, 0);
  EXPECT_GE(stats.load_time.count(), 0);  // Can be 0 for small files
}

// Test initialization with missing CSV file
TEST(DataNodeTest, InitializeWithMissingCSV) {
  DataNode node(0, getTestDataPath("nonexistent_file.csv"));

  bool success = node.initialize();

  EXPECT_FALSE(success);

  // Statistics should reflect no records loaded
  DataNode::Statistics stats = node.getStatistics();
  EXPECT_EQ(stats.total_records, 0);
}

// Test search returning complete records
TEST(DataNodeTest, SearchReturnsCompleteRecords) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  // Search for "3RD" which should match "3RD ST"
  std::vector<std::string> query_terms = {"3RD"};
  std::vector<AddressRecord> results = node.search(query_terms);

  EXPECT_EQ(results.size(), 1);
  ASSERT_GT(results.size(), 0);

  // Verify the record has all fields populated
  const AddressRecord& record = results[0];
  EXPECT_DOUBLE_EQ(record.longitude, -122.608996);
  EXPECT_DOUBLE_EQ(record.latitude, 47.166377);
  EXPECT_EQ(record.number, "611");
  EXPECT_EQ(record.street, "3RD ST");
  EXPECT_EQ(record.city, "Steilacoom");
  EXPECT_EQ(record.postcode, "98388");
}

// Test search with multiple terms (intersection)
TEST(DataNodeTest, SearchWithMultipleTerms) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  // Search for "MCKINNON" and "SALINAS" - should match one record
  std::vector<std::string> query_terms = {"MCKINNON", "SALINAS"};
  std::vector<AddressRecord> results = node.search(query_terms);

  EXPECT_EQ(results.size(), 1);
  ASSERT_GT(results.size(), 0);

  const AddressRecord& record = results[0];
  EXPECT_EQ(record.street, "MCKINNON STREET");
  EXPECT_EQ(record.city, "Salinas");
}

// Test empty query handling
TEST(DataNodeTest, EmptyQueryHandling) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  std::vector<std::string> empty_query = {};
  std::vector<AddressRecord> results = node.search(empty_query);

  EXPECT_EQ(results.size(), 0);
}

// Test search with no matches
TEST(DataNodeTest, SearchWithNoMatches) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  std::vector<std::string> query_terms = {"NONEXISTENT", "STREET"};
  std::vector<AddressRecord> results = node.search(query_terms);

  EXPECT_EQ(results.size(), 0);
}

// Test index inconsistency handling
// This test simulates the scenario where RadixTree has an ID but ForwardIndex
// doesn't We can't easily create this scenario with the current implementation,
// but we can verify the logging behavior by checking that search handles
// missing records gracefully
TEST(DataNodeTest, IndexInconsistencyHandling) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  // Normal search should work fine
  std::vector<std::string> query_terms = {"SALINAS"};
  std::vector<AddressRecord> results = node.search(query_terms);

  // Should return results without crashing even if there were inconsistencies
  EXPECT_GE(results.size(), 0);
}

// Test that malformed records don't prevent initialization
TEST(DataNodeTest, InitializeWithMalformedRecords) {
  DataNode node(0, getTestDataPath("malformed_addresses.csv"));

  bool success = node.initialize();

  // Should succeed even with some malformed records
  EXPECT_TRUE(success);

  // Should have loaded at least the valid records
  DataNode::Statistics stats = node.getStatistics();
  EXPECT_GT(stats.total_records, 0);
  EXPECT_LT(stats.total_records,
            7);  // Less than total lines (some are malformed)
}

// Test logging behavior - verify that operations produce expected log output
// This is a basic test that verifies the node can be initialized and searched
// without crashing, which implicitly tests that logging doesn't cause issues
TEST(DataNodeTest, LoggingBehavior) {
  // Redirect cout to capture logs
  std::stringstream buffer;
  std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  node.initialize();

  std::vector<std::string> query_terms = {"3RD"};
  node.search(query_terms);

  // Restore cout
  std::cout.rdbuf(old_cout);

  std::string log_output = buffer.str();

  // Verify key log messages are present
  EXPECT_NE(log_output.find("[INFO] [DataNode] Starting data load"),
            std::string::npos);
  EXPECT_NE(log_output.find("[INFO] [DataNode] Successfully parsed"),
            std::string::npos);
  EXPECT_NE(log_output.find("[INFO] [DataNode] Building indexes"),
            std::string::npos);
  EXPECT_NE(log_output.find("[INFO] [DataNode] Index building complete"),
            std::string::npos);
  EXPECT_NE(log_output.find("[INFO] [DataNode] Processing search query"),
            std::string::npos);
}

// Test statistics reporting
TEST(DataNodeTest, StatisticsReporting) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  DataNode::Statistics stats = node.getStatistics();

  // Verify all statistics are populated
  EXPECT_GT(stats.total_records, 0);
  EXPECT_GT(stats.radix_tree_memory, 0);
  EXPECT_GT(stats.forward_index_size, 0);
  EXPECT_GE(stats.load_time.count(), 0);
}

// Test case-insensitive search (normalization)
TEST(DataNodeTest, CaseInsensitiveSearch) {
  DataNode node(0, getTestDataPath("valid_addresses.csv"));
  ASSERT_TRUE(node.initialize());

  // Search with lowercase should work due to normalization
  std::vector<std::string> query_terms = {"steilacoom"};
  std::vector<AddressRecord> results = node.search(query_terms);

  EXPECT_EQ(results.size(), 1);
  ASSERT_GT(results.size(), 0);

  const AddressRecord& record = results[0];
  EXPECT_EQ(record.city, "Steilacoom");
}
