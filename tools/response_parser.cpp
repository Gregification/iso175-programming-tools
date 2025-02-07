//run localy or in some online compiler
// formats the data for the set command.
#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <cassert>
#include <map>

// from https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_SAEJ1939_D00415_N_XXEN.pdf
//  section 1.5.1
union __attribute__((__packed__)) PGN_Response {
  unsigned long long raw : 64;
  uint8_t data[8];

  struct __attribute__((__packed__)) {
    uint8_t index;

    union __attribute__((__packed__)) {
      uint32_t dataWord32;
      uint16_t dataWord16[3];
    };
  };
};
static_assert(sizeof(PGN_Response) == 8, "incorrect struct size");

//this is not a complete list. section 1.5.1(N)
std::map<uint8_t, std::string> index_info = {
  {0x0a, "Bootloader Identifi-cation: Build number"},
  {0x0c, "Bootloader Identifi-cation: D-Number"},
  {0x0e, "Bootloader Identifi-cation: Version"},
  //...
  {0x2e, "Unbalance: Alarm Threshold "},
};

int main()
{
  PGN_Response response;
  response.raw = 0x888FEB6080001FF; // note that if pasting raw, the byte after the id is the length, not part of the data
  
  // printing
  auto asint = *reinterpret_cast<uint32_t*>(&response);
  std::cout << "raw: 0x" << std::setfill('0') << std::setw(3*2) << std::hex << asint << std::endl;
  std::cout << "index: 0x" << std::hex << (long)response.index << std::endl;
  std::cout << "index info:" << (index_info.count(response.index) ? index_info[response.index] : "(unkown)") << std::endl;
  std::cout << "data[0]: " << std::dec << (long)response.data[0] << std::endl;
  std::cout << "data[1]: " << std::dec << (long)response.data[1] << std::endl;
  std::cout << "data[2]: " << std::dec << (long)response.data[2] << std::endl;
  std::cout << "data[3]: " << std::dec << (long)response.data[3] << std::endl;
  std::cout << "data[4]: " << std::dec << (long)response.data[4] << std::endl;
  std::cout << "data[5]: " << std::dec << (long)response.data[5] << std::endl;
  std::cout << "data[6]: " << std::dec << (long)response.data[6] << std::endl;
  std::cout << "data[7]: " << std::dec << (long)response.data[7] << std::endl;

  // meta
  std::cout << std::endl;
  std::cout << "meta ... iso175 section 1.5.1" << std::endl;
  std::cout << "dataWord16[0]: " << std::dec << (long)response.dataWord16[0] << std::endl;
  std::cout << "dataWord16[1]: " << std::dec << (long)response.dataWord16[1] << std::endl;
  std::cout << "dataWord16[2]: " << std::dec << (long)response.dataWord16[2] << std::endl;
  std::cout << "dataWord32: " << std::dec << (long)response.dataWord32 << std::endl;

  return 0;
}