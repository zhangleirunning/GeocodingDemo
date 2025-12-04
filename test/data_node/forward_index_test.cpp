// Forward Index Unit Tests

#include <gtest/gtest.h>

#include "data_node/forward_index.h"

TEST(ForwardIndexTest, InsertAndGet) {
  ForwardIndex index;

  AddressRecord record;
  record.longitude = -122.608996;
  record.latitude = 47.166377;
  record.hash = 0x1234567890ABCDEF;
  record.number = "611";
  record.street = "3RD STREET";
  record.unit = "";
  record.city = "STEILACOOM";
  record.postcode = "98388";
  record.original_street = "3rd St";
  record.original_unit = "";
  record.original_city = "Steilacoom";

  index.insert(record.hash, record);

  auto retrieved = index.get(record.hash);
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->longitude, record.longitude);
  EXPECT_EQ(retrieved->latitude, record.latitude);
  EXPECT_EQ(retrieved->hash, record.hash);
  EXPECT_EQ(retrieved->number, record.number);
  EXPECT_EQ(retrieved->street, record.street);
  EXPECT_EQ(retrieved->city, record.city);
}

TEST(ForwardIndexTest, GetNonExistent) {
  ForwardIndex index;

  auto result = index.get(0x9999999999999999);
  EXPECT_FALSE(result.has_value());
}

TEST(ForwardIndexTest, Contains) {
  ForwardIndex index;

  AddressRecord record;
  record.hash = 0xABCDEF1234567890;
  record.city = "SEATTLE";

  index.insert(record.hash, record);

  EXPECT_TRUE(index.contains(record.hash));
  EXPECT_FALSE(index.contains(0x9999999999999999));
}

TEST(ForwardIndexTest, RecordCount) {
  ForwardIndex index;

  EXPECT_EQ(index.getRecordCount(), 0);

  AddressRecord record1;
  record1.hash = 0x1111111111111111;
  index.insert(record1.hash, record1);

  EXPECT_EQ(index.getRecordCount(), 1);

  AddressRecord record2;
  record2.hash = 0x2222222222222222;
  index.insert(record2.hash, record2);

  EXPECT_EQ(index.getRecordCount(), 2);
}

TEST(ForwardIndexTest, StorageSize) {
  ForwardIndex index;

  // Empty index should have some base size
  size_t empty_size = index.getStorageSize();
  EXPECT_GT(empty_size, 0);

  AddressRecord record;
  record.hash = 0xABCDEF1234567890;
  record.city = "SEATTLE";
  index.insert(record.hash, record);

  // Size should increase after insertion
  size_t size_with_record = index.getStorageSize();
  EXPECT_GT(size_with_record, empty_size);
}
