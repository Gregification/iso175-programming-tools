//run localy or in some online compiler
// encode/decodes the can ID
// IMPORTANT see here for J1939 gimics & troubleshooting : https://www.typhoon-hil.com/documentation/typhoon-hil-software-manual/References/j1939_protocol.html
#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <cassert>

// from https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_SAEJ1939_D00415_N_XXEN.pdf
// and figure 2 of https://www.typhoon-hil.com/documentation/typhoon-hil-software-manual/References/j1939_protocol.html
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

#define PGN_REQUEST_TX 61184
//18ff01f4
int main()
{
    SAEJ1939CANID29 id;

    //encode id
    id.priority = 0b111;// arbitrary value. the can controller will send it whenever.
    id.srcAddr = 0xF4;  // must be a unique adderss on the network.
    id.PGN = PGN_REQUEST_TX;
    // id.PDU_format = 238; // <240 -> ptp
    // id.PDU_specific = 255; // 255 to broadcast in ptp
    
    //decode id
    // id.raw = 0x1cebfff4;
    
    assert(id.padding_must_be_zero == 0); // the padding bits must be 0
    
    // printing
    auto asint = *reinterpret_cast<uint32_t*>(&id);
    std::cout << "raw ID: 0x" << std::setfill('0') << std::setw(sizeof(asint)*2) << std::hex << asint << std::endl;
    std::cout << "src addr: " << std::dec << (long)id.srcAddr << std::endl;
    std::cout << "priority: " << std::dec << (long)id.priority << std::endl;
    std::cout << "pgn: " << std::dec << (long)id.PGN << std::endl;
    std::cout << "entire pdu: " << std::dec << (long)((id.PDU_format << 8) | id.PDU_specific) << std::endl;
    std::cout << "pdu format: " << std::dec << (long)id.PDU_format << std::endl;
    std::cout << "pdu specific: " << std::dec << (long)id.PDU_specific << std::endl;
    std::cout << "edp: " << std::dec << (long)id.EDP << std::endl;
    std::cout << "dp: " << std::dec << (long)id.DP << std::endl;

    // printing meta
    std::cout << std::endl;
    std::cout << "meta ... " << std::endl;
    std::cout << "PGN : ";
    switch(id.PGN){
      case 60671: std::cout << "Indicates the current engine oil pressure value" <<std::endl; break;
      case 61184: std::cout << "PGN_Request (read)" << std::endl; break;
      case 61428: std::cout << "PGN_Request (control)" << std::endl; break;
      case 65281: std::cout << "PGN_Info_General" << std::endl; break;
      case 65282: std::cout << "PGN_Info_IsolationDetail" << std::endl; break;
      case 65283: std::cout << "PGN_Info_Voltage" << std::endl; break;
      case 65284: std::cout << "PGN_Info_IT-System" << std::endl; break;
      case 65280: std::cout << "a \"proprietary\" message" << std::endl; break;
      default: std::cout << "(unknown " << std::dec << id.PGN << ") " << std::endl; break;
    };
    std::cout << "PDU1/peer-to-peer: " << std::boolalpha << (id.PDU_format < 240)  << std::endl;
    std::cout << "    PDU1 > broadcast: " << std::boolalpha << (id.PDU_specific == 255)  << std::endl;
    std::cout << "PDU2/broadcast: " << std::boolalpha << (id.PDU_format >= 240)  << std::endl;
    //std::cout << ": " << std::boolalpha << ()  << std::endl;

    return 0;
}