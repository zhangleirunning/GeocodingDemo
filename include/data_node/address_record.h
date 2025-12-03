#ifndef DATA_NODE_ADDRESS_RECORD_H_
#define DATA_NODE_ADDRESS_RECORD_H_

#include <cmath>
#include <string>

struct AddressRecord {
  double longitude;
  double latitude;
  std::string hash;
  std::string number;
  std::string street;
  std::string unit;
  std::string city;
  std::string postcode;

  // Original unnormalized values for display
  std::string original_street;
  std::string original_unit;
  std::string original_city;

  // Default constructor
  AddressRecord() : longitude(0.0), latitude(0.0) {}

  // Constructor with all fields
  AddressRecord(double longitude_,
                double latitude_,
                const std::string& hash_,
                const std::string& number_,
                const std::string& street_,
                const std::string& unit_,
                const std::string& city_,
                const std::string& postcode_,
                const std::string& original_street_,
                const std::string& original_unit_,
                const std::string& original_city_)
      : longitude(longitude_),
        latitude(latitude_),
        hash(hash_),
        number(number_),
        street(street_),
        unit(unit_),
        city(city_),
        postcode(postcode_),
        original_street(original_street_),
        original_city(original_city_),
        original_unit(original_unit_) {}

  // Equality operator
  bool operator==(const AddressRecord& other) const {
    // Compare floating point coordinates with epsilon tolerance
    const double kEpsilon = 1e-9;
    bool coords_equal = std::abs(longitude - other.longitude) < kEpsilon &&
                        std::abs(latitude - other.latitude) < kEpsilon;

    return coords_equal &&
           hash == other.hash &&
           number == other.number && street == other.street &&
           unit == other.unit && city == other.city &&
           postcode == other.postcode &&
           original_street == other.original_street &&
           original_city == other.original_city &&
           original_unit == other.original_unit;
  }

  // Inequality operator
  bool operator!=(const AddressRecord& other) const { return !(*this == other); }
};

#endif  // DATA_NODE_ADDRESS_RECORD_H_
