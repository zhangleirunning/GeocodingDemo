#ifndef ADDRESS_RECORD_H
#define ADDRESS_RECORD_H

#include <string>

struct AddressRecord {
    std::string id;
    std::string hash;
    double longitude;
    double latitude;
    std::string number;
    std::string street;
    std::string unit;
    std::string city;
    std::string district;
    std::string region;
    std::string postcode;
    
    // Original unnormalized values for display
    std::string original_street;
    std::string original_city;
    std::string original_district;
    std::string original_region;
    std::string original_postcode;
};

#endif // ADDRESS_RECORD_H
