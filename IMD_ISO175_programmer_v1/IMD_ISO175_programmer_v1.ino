/*
 * purpose: to program and verify settings on the ISO175
 * host board: adafruit feather m4 can (same51)
 * datasheets: from here https://www.benderinc.com/products/ground-fault-monitoring-ungrounded/isometer-iso175/
 * 
 * this code is wonky. this should be connceted by CAN bus to the ISO175 only, nothin else should be on CAN.
 * - note that the iso175 comes preconfigured from factory to use either standard CAN or J1939, there are seperate can messages for both. this program is for J1939
 *    here are the J1939 specs. https://www.benderinc.com/fileadmin/content/Products/a/e/iso175_SAEJ1939_D00415_N_XXEN.pdf
 * - setup instructions:
 *    - only the arduino and the iso175 should be connected on CAN.
 *    - not required but would suggest not having HV on the iso175 just in case.
 *    - there may be a ground difference between your laptop powering the arduino and the iso175's power supply,
 *        directly connecting the grounds may over current the usb port, try jumping with a resistor (I eyeballed 130ohm and it seems to work).
 *    1. the iso175 powered off. the arduino powered on.
 *    2. restart the arduino.
 *    3. power on the iso175.
 *    4. watch arduino serial output for info on what its doing.
 *    5. power off iso175 when done.
 * - this is simple linear code, it has no loops.
 */

//e.g: 250k baud = 250000
#define CAN_SPEED 500000

// #define DEBUG_DEVICEINFO_ONSTART

#include "ISO175_v1.h"
#include <stdint.h>

// adafruit library. using version 0.2.1
#include <CANSAME5x.h>
CANSAME5x CAN;

void printDeviceInfoOnStart();

void sendRequest(uint8_t);

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);
  Serial.println();
  Serial.println("IMD_ISO175_programmer_v1");

  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, false);  // turn off STANDBY

  pinMode(PIN_CAN_BOOSTEN, OUTPUT);
  digitalWrite(PIN_CAN_BOOSTEN, true);  // turn on booster

  if (!CAN.begin(CAN_SPEED)) {
    Serial.println("Starting CAN failed! restart the program to try again");
    while (1) delay(10);
  }

  // for(int i = 0; i <=7 ; i++)
  {
    Serial.println("sending");

    SAEJ1939CANID29 id;
    // id.srcAddr = 0xF4;
    // id.priority = 0b111;
    // id.PDU_format = 238;
    // id.PDU_specific = 255;
    // id.EDP = 0;
    // id.DP = 0;
    
    id.raw = 0x18Eff417;

    uint32_t tmp = *reinterpret_cast<uint32_t*>(&id);

    // CAN.beginExtendedPacket(tmp,8);
    
    // CAN.write(0x66);
    // CAN.write(0xff);
    // CAN.write(0xff);
    // CAN.write(0xff);

    // CAN.write(0xff);
    // CAN.write(0xff);
    // CAN.write(0xff);
    // CAN.write(0xff);

    // CAN.endPacket();
  }

  // printDeviceInfoOnStart();
}

void loop() {
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    if(CAN.packetId() == 0x18ff01f4)
      return;

    // received a packet
    Serial.print("Received ");

    if (CAN.packetExtended()) {
      Serial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      Serial.print("RTR ");
    }

    Serial.print("packet with id 0x");
    Serial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      Serial.print(" and requested length ");
      Serial.println(CAN.packetDlc());
    } else {
      Serial.print(" (length: ");
      Serial.print(packetSize);
      Serial.print(" ) : 0x");

      // only print packet data for non-RTR packets
      while (CAN.available()) {
        Serial.print(CAN.read(), HEX);
        Serial.print(' ');
      }
      Serial.println();
    }
  }
}
// void ISO175txCAN(Response tx, uint8_t size) {
//   CAN.beginPacket(0x12);

//   for (int i = 0; i < size; i++)
//     CAN.write(tx.raw_arr[i]);

//   CAN.endPacket();
// }

void sendRequest(uint8_t idx) {
  SAEJ1939CANID29 id;
  id.PGN = 61184;
  id.srcAddr = 0x5A;
  id.priority = 0b100;

  Serial.println("sizeof: ");
  Serial.println(sizeof(SAEJ1939CANID29));

  uint32_t u = *reinterpret_cast<uint32_t*>(&id);

  Serial.print("can id tx: 0x");
  Serial.println(u, HEX);

  CAN.beginPacket(0x00ef002c, 8, false);
  CAN.write(idx);
  CAN.endPacket();
};

// slap job, it works though.
// - the string values being non printable characters is probably a bug with this function. if you turn on debug youll see the correct data was sent, just bad parsing on my end.
void printDeviceInfoOnStart() {

  TPDeviceInfo tpcm_bam;
  TPDeviceInfo buf;
  TPData data;

  int tries = 20, lastPacket = 0;
  for (; tries >= 0; tries--) {
    if (lastPacket == 6)
      break;

    int pktsize;
    while ((pktsize = CAN.parsePacket()) == 0 || CAN.packetRtr())
      ;

    buf.CAN_ID = CAN.packetId();

#ifdef DEBUG_DEVICEINFO_ONSTART
    Serial.print("printDeviceInfoOnStart > received (size:");
    Serial.print(pktsize);
    Serial.print("): id 0x");
    Serial.println(buf.CAN_ID, HEX);
#endif

    if (pktsize != 8) {
#ifdef DEBUG_DEVICEINFO_ONSTART
      Serial.println("    unexpected message, ignoring. (expected length: 8 Bytes)");
#endif

      continue;
    }

    CAN.readBytes(buf.byte_arr, 8);

    //transport parsing
    bool targ = false;
    if (buf.CAN_ID == ISO175_TPCM_BAM_CANID) {
      targ = true;
      tpcm_bam = buf;

    } else if (buf.CAN_ID == ISO175_TPDT_MESSAGE) {
      targ = true;
      if (buf.DT.packetNum > 6) {  // if is invalid packet number
#ifdef DEBUG_DEVICEINFO_ONSTART
        Serial.print("    unexpected packet number: 0x");
        Serial.print(buf.DT.packetNum, HEX);
        Serial.println();
#endif
      } else {
#ifdef DEBUG_DEVICEINFO_ONSTART
        Serial.print("    packet number: 0x");
        Serial.print(buf.DT.packetNum, HEX);
        Serial.println();
#endif

        lastPacket = buf.DT.packetNum;

        for (uint8_t i = 0; i < 8; i++) {
          size_t index = buf.DT.packetNum * 8 + i;
          if (index < sizeof(data))  // if is valid index, this should only aply to the last packet where its padded
            data.data[index] = buf.DT.data[i];
        }
      }

    } else {
#ifdef DEBUG_DEVICEINFO_ONSTART
      Serial.println("    unexpected message, ignoring. (expected id: 0x1CECFFF4 or 0x1CEBFFF4)");
#endif
    }

    if (targ) {
#ifdef DEBUG_DEVICEINFO_ONSTART
      Serial.print("    message : ");
      for (int k = 0; k < sizeof(buf.byte_arr); k++) {
        Serial.print(buf.byte_arr[k], HEX);
      }
      Serial.print("    as str:    ");
      for (int k = 0; k < sizeof(buf.byte_arr); k++) {
        Serial.print((char)(buf.byte_arr[k]));
      }
      Serial.println();
#endif
    }
  }

  Serial.println();
  if (tries <= 0)
    Serial.println("tried too many times");

  Serial.println("Received Device Info message content");
  Serial.print("TP.CM_BAM > broadcase (expected 0x20):");
  Serial.println(tpcm_bam.CM_BAM.broadcase, HEX);
  Serial.print("TP.CM_BAM > total num bytes (expected 42):");
  Serial.println(tpcm_bam.CM_BAM.numBytes);
  Serial.print("TP.CM_BAM > num packets:");
  Serial.println(tpcm_bam.CM_BAM.numPackets);
  Serial.print("TP.CM_BAM > byte 5 (expected 0xFF):");
  Serial.println(tpcm_bam.CM_BAM.reserved, HEX);
  Serial.print("TP.CM_BAM > PGN (expected 65280):");
  Serial.println(tpcm_bam.CM_BAM.PGN);

  Serial.print("TP.DT > device type:");
  Serial.write(data.deviceType, 14);
  Serial.println();
  Serial.print("TP.DT > serial num:");
  Serial.write(data.serialNum, 14);
  Serial.println();
  Serial.print("TP.DT > part num:");
  Serial.write(data.partNum, 14);
  Serial.println();

  Serial.println();
}
