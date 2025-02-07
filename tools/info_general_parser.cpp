//run localy or in some online compiler
// parses the data for a PGN_Info_General message

#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <cassert>
#include <map>

union __attribute__((__packed__)) SAEJ1939CANID29 {
  struct __attribute__((__packed__)) {
    unsigned int raw : 29;
    unsigned int padding_must_be_zero : 3;
  };

  struct __attribute__((__packed__)) {
    uint8_t srcAddr;

    union __attribute__((__packed__)) {
      
      struct __attribute__((__packed__)) {
        unsigned int PGN : 18;
        unsigned int priority : 3;
      };

      struct __attribute__((__packed__)) {
        uint8_t PDU_specific;
        uint8_t PDU_format;
        unsigned int DP : 1; // data page
        unsigned int EDP : 1; // reserved. the iso175 calls it "EDP"
        unsigned int : 3; // refrences the same bit space as priority 
      };
    };
  };
};
static_assert(sizeof(SAEJ1939CANID29) == 4, "incorrect struct size ");

// from https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_SAEJ1939_D00415_N_XXEN.pdf
//  section 1.5.1
union __attribute__((__packed__)) PGN_data_info_general {
  unsigned long long raw : 64;
  uint8_t data[8];

  struct __attribute__((__packed__)) {
    uint16_t corrected_isolation_value;
    uint8_t status;
    uint8_t measurement_counter;
    uint16_t warnings_and_alarms;
    uint8_t device_activity;
    uint8_t data7;
  };
};
static_assert(sizeof(PGN_data_info_general) == 8, "incorrect struct size");

union __attribute__((__packed__)) J1939_8_Packet {
  __uint128_t raw : 101; // 29b(ID) + 1B(len) + 8B(data)

  struct __attribute__((__packed__)) {
      SAEJ1939CANID29 ID;
      uint8_t length;
      PGN_data_info_general data;
  };
};
// 4B(id) + 1B(len) + 8B(data)
static_assert(sizeof(J1939_8_Packet) == (4 + 1 + 8), "incorrect struct size");

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
  J1939_8_Packet pk;
  // pk.raw is incorrect, dont use it until you fix the warning. im not enturely sure how to go about that; cant figure out hte syntas for <long long> decelerations

  pk.ID.raw = 0x18FF01F4;
  pk.length = 8;
  pk.data.raw = 0xff010000f5fe88b8;

  // printing

  // std::cout << "raw: 0x" << std::setfill('0') << std::setw(sizeof(pk)) << std::hex << (uint64_t)(pk.raw) << std::endl;
  std::cout << "id: 0x" << std::hex << pk.ID.raw << std::endl;
  std::cout << "id PGN: " << std::dec << pk.ID.PGN << std::endl;
  std::cout << "data length: " << std::dec << pk.length << std::endl; 
  std::cout << "raw data: 0x" << std::hex << pk.data.raw << std::endl; 
  std::cout << "data[7] (expected 0xFF or N/A): 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<long long>(pk.data.data7) << std::endl;

  // meta
  std::cout << std::endl;
  std::cout << "is PGN_Info_General ID : " << std::boolalpha << (pk.ID.PGN == 65281) << std::endl;
  std::cout << "meta ... assuming it is a PGN_Info_General message" << std::endl;
  std::cout << "    corrected isolation value (kOhm): ";
  if(pk.data.corrected_isolation_value == 0xFFFF)
    std::cout << std::dec << pk.data.corrected_isolation_value << std::endl;
  else
    std::cout << "SNV" << std::endl;
  std::cout << "    status : ";
  switch(pk.data.status){
    case 0xFC : std::cout << "estemiated isolation value during startup" << std::endl; break;
    case 0xFD : std::cout << "first measured isolation value during startup" << std::endl; break;
    case 0xFE : std::cout << "isolation value in normal operation" << std::endl; break;
    case 0xFF : std::cout << "SNV" << std::endl; break;
    default: std::cout << "(unexpected value : 0x" << std::hex << static_cast<long long>(pk.data.status) << ")" << std::endl; break;
  }
  std::cout << "    measurement counter : " << std::dec << static_cast<long long>(pk.data.measurement_counter) << std::endl;
  std::cout << "    warnings and alarms :" << std::endl;
  //womp womp
  if(pk.data.warnings_and_alarms & (1 << 0))
    std::cout << "        device error active" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 1))
    std::cout << "        HV_pos connection failure" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 2))
    std::cout << "        HV_neg connection failure" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 3))
    std::cout << "        Earth connection failure" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 4))
    std::cout << "        Iso alarm (iso value bellow thresh error)" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 5))
    std::cout << "        Iso warning (iso value bellow thresh warn)" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 6))
    std::cout << "        Iso outdated (value „Time elapsed since lst measurement“ > = „measurement timeout“)" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 6))
    std::cout << "        Unbalance Alarm (unbalance value below threshold)" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 8))
    std::cout << "        Undervoltage alarm" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 9))
    std::cout << "        Unsafe to start" << std::endl;
  if(pk.data.warnings_and_alarms & (1 << 10))
    std::cout << "        Earthlift open" << std::endl; // whar? i assume earth not connected?
  if(pk.data.warnings_and_alarms == 0)
    std::cout << "        No warnings or alarms" << std::endl;
  std::cout << "    device activity : " << std::endl;
  switch(pk.data.device_activity){
    case 0 : std::cout << "        initialization" << std::endl; break;
    case 1 : std::cout << "        normal operation" << std::endl; break;
    case 2 : std::cout << "        self test" << std::endl; break;
    default: std::cout << "        (unexpected value : " << pk.data.device_activity << ")" << std::endl; break;
  }

  return 0;
}