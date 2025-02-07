/*
  can messages for the iso175
  only some are implimented, feel free to expand this
  datasheet: https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_CAN_D00415_N_XXEN.pdf

  notes:
    - J1939 can bus
    - message lengths of Requests are not fixed, however unused bytes have to be filled up with 0xFF
    - put the packed attribute on everything to make it brain dead to understand even if its a eye sore.
    - this file is the least updated, see the deticated files in the tools folder for corrected structs
*/
#ifndef __ISO175_v1_H
#define __ISO175_v1_H

#include <stdint.h>
#include <cassert>

// can message, see section 1.3.1
//  - byte_arr index has a +1 offset
struct __attribute__((__packed__)) TPDeviceInfo {
  uint32_t CAN_ID;

  union {
    uint8_t byte_arr[8];

    union __attribute__((__packed__)) {
      struct __attribute__((__packed__)) {
        uint8_t broadcase;
        uint16_t numBytes;
        uint8_t numPackets;
        uint8_t reserved;
        unsigned int PGN : 24;  // 3 bytes. stack-overflow says uint24_t is not universally supported.
      } CM_BAM;

      struct __attribute__((__packed__)) {
        uint8_t packetNum;
        uint8_t data[7];
      } DT;
    };
  };
};
// assert(sizeof(TPDeviceInfo) == (32+8)); // if struct packing was correctly enabled

union __attribute__((__packed__)) TPData {
  uint8_t data[42];
  struct __attribute__((__packed__)) {
    char deviceType[14];
    char serialNum[14];
    char partNum[14];  // i THINK this is supposed to be ascii; remember seeing this on the datasheet but idk where
  };
};

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
        unsigned int DP : 1;
        unsigned int EDP : 1;
        unsigned int : 3; // refrences the same bit space as priority 
      };
    };
  };
};
// oops, this fails!  idk y, i really should fix it
static_assert(sizeof(SAEJ1939CANID29) == 4, "non standard size");

//
// struct __attribute__((__packed__)) PGN_Request_index {
//   unsigned int : 1; //paddingv
//   unsigned int index : 7;
// };

union __attribute__((__packed__)) PGN_Request_Resp {

  uint8_t data[8];

  struct __attribute__((__packed__)){
    uint8_t index;

    union __attribute__((__packed__)){
      uint16_t dataWord16[3];
      uint32_t dataWord32;
    };
  };
};

#define ISO175_TPCM_BAM_CANID 0x1CECFFF4
#define ISO175_TPDT_MESSAGE 0x1CEBFFF4

#define ISO175_IMD_Request 0x22
#define ISO175_IMD_Request_B1_invalidRequest 0x23
#define ISO175_IMD_Request_B1_setCmdFailParaLocked 0x24

#define ISO175_IMD_Response 0x23

#endif /* __ISO175_v1_H */