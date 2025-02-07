//run localy or in some online compiler

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <cassert>

// from https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_SAEJ1939_D00415_N_XXEN.pdf
//  section 1.5.1
union __attribute__((__packed__)) PGN_Request_Data {
    uint8_t raw_index; // the datasheet lists this value
    struct __attribute__((__packed__)) { // the bits are in this order
        unsigned int padding : 1; //padding
        unsigned int index : 7;
    };
};
static_assert(sizeof(PGN_Request_Data) == 1, "incorrect struct size");

int main()
{
    PGN_Request_Data data;
    data.raw_index = 0xf2; // desired value
    
    assert(data.padding == 0); // first bit should be 0

    // printing
    auto asint = *reinterpret_cast<uint8_t*>(&data);
    std::cout << "data byte: 0x" << std::setfill('0') << std::setw(sizeof(asint)*2) << std::hex << asint << std::endl;

    return 0;
}