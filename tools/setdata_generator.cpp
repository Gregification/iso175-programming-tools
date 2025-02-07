//run localy or in some online compiler
// formats the data for the set command.
#include <iostream>
#include <stdint.h>
#include <iomanip>
#include <cassert>

// from https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_SAEJ1939_D00415_N_XXEN.pdf
//  section 1.1
union __attribute__((__packed__)) Set_Data {
  uint8_t data[3];
  struct __attribute__((__packed__)) {
    uint8_t index;
    uint16_t dataWord1;
  };
};
static_assert(sizeof(Set_Data) == 3, "incorrect struct size");

#define IDX_THREASHOLD_UNDERVOLTAGE     0x67
#define IDX_UNBALANCE_THRSHOLD          0x2F
#define IDX_SELF_HOLDING_ISO_ALARM_ACTI 0x31

int main()
{
  Set_Data data;
  data.index = IDX_THREASHOLD_UNDERVOLTAGE;
  data.dataWord1 = 300;
  //0x012c67

  assert(data.index >= 0x2F && data.index <= 0x75);
  
  // printing
  auto asint = *reinterpret_cast<uint32_t*>(&data);
  std::cout << "raw: 0x" << std::setfill('0') << std::setw(3*2) << std::hex << asint << std::endl;
  std::cout << "data0/index: " << std::hex << (long)data.data[0] << std::endl;
  std::cout << "data1/AA: " << std::hex << (long)data.data[1] << std::endl;
  std::cout << "data2/BB: " << std::hex << (long)data.data[2] << std::endl;
  std::cout << "dataWord1: " << std::hex << (long)data.dataWord1 << std::endl;

  return 0;
}