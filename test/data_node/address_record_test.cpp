#include "data_node/address_record.h"

#include <gtest/gtest.h>

TEST(AddressRecordTest, DefaultConstructor) {
  AddressRecord record;
  EXPECT_EQ(record.longitude, 0.0);
  EXPECT_EQ(record.latitude, 0.0);
  EXPECT_EQ(record.hash, 0);
}

TEST(AddressRecordTest, ParameterizedConstructor) {
  AddressRecord record(-122.608996, 47.166377, 0x668f4a26abdd476d, "611", "3RD STREET",
                       "APT 5", "STEILACOOM", "98388", "3rd St", "Apt 5",
                       "Steilacoom");

  EXPECT_DOUBLE_EQ(record.longitude, -122.608996);
  EXPECT_DOUBLE_EQ(record.latitude, 47.166377);
  EXPECT_EQ(record.hash, 0x668f4a26abdd476d);
  EXPECT_EQ(record.number, "611");
  EXPECT_EQ(record.street, "3RD STREET");
  EXPECT_EQ(record.unit, "APT 5");
  EXPECT_EQ(record.city, "STEILACOOM");
  EXPECT_EQ(record.postcode, "98388");
  EXPECT_EQ(record.original_street, "3rd St");
  EXPECT_EQ(record.original_unit, "Apt 5");
  EXPECT_EQ(record.original_city, "Steilacoom");
}

TEST(AddressRecordTest, EqualityOperator) {
  AddressRecord record1(-122.608996, 47.166377, 0x668f4a26abdd476d, "611",
                        "3RD STREET", "", "STEILACOOM", "98388", "3rd St", "",
                        "Steilacoom");

  AddressRecord record2(-122.608996, 47.166377, 0x668f4a26abdd476d, "611",
                        "3RD STREET", "", "STEILACOOM", "98388", "3rd St", "",
                        "Steilacoom");

  EXPECT_EQ(record1, record2);
}

TEST(AddressRecordTest, InequalityOperator) {
  AddressRecord record1(-122.608996, 47.166377, 0x668f4a26abdd476d, "611",
                        "3RD STREET", "", "STEILACOOM", "98388", "3rd St", "",
                        "Steilacoom");

  AddressRecord record2(-122.608996, 47.166377, 0xABCDEF1234567890, "611",
                        "3RD STREET", "", "STEILACOOM", "98388", "3rd St", "",
                        "Steilacoom");

  EXPECT_NE(record1, record2);
}

TEST(AddressRecordTest, FloatingPointTolerance) {
  AddressRecord record1(-122.608996, 47.166377, 0x668f4a26abdd476d, "611",
                        "3RD STREET", "", "STEILACOOM", "98388", "3rd St", "",
                        "Steilacoom");

  // Slightly different coordinates within epsilon tolerance
  AddressRecord record2(-122.608996 + 1e-10, 47.166377 + 1e-10, 0x668f4a26abdd476d,
                        "611", "3RD STREET", "", "STEILACOOM", "98388",
                        "3rd St", "", "Steilacoom");

  EXPECT_EQ(record1, record2);
}
