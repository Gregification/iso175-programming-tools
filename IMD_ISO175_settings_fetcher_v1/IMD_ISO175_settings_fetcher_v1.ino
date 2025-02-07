/*
 * purpose: to program and verify settings on the ISO175
 * host board: adafruit feather m4 can (same51), aka the offbrand arduino nano with a built in CAN adaptor
 * 
 * - this does not grab all data, add it if you want something moew.
 * yap yap yap yap, go read the comments in the other arduino program for more info. this was baselined off that one. im losing it, its week 2 of the few hour project.
 */

//e.g: 250k baud = 250000
#define CAN_SPEED 500000

// #define DEBUG_DEVICEINFO_ONSTART

#include <stdint.h>

// adafruit library. using version 0.2.1
#include <CANSAME5x.h>
CANSAME5x CAN;

union __attribute__((__packed__)) PGN_Request_Resp {
  unsigned long long raw : 64;
  uint8_t data[8];

  struct __attribute__((__packed__)){
    uint8_t index;
    uint16_t dataWord[3];
  };
};

PGN_Request_Resp request(uint8_t index);

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);
  Serial.println();
  Serial.println("IMD_ISO175_settings_fetcher_v1");

  pinMode(PIN_CAN_STANDBY, OUTPUT);
  digitalWrite(PIN_CAN_STANDBY, false);  // turn off STANDBY

  pinMode(PIN_CAN_BOOSTEN, OUTPUT);
  digitalWrite(PIN_CAN_BOOSTEN, true);  // turn on booster

  if (!CAN.begin(CAN_SPEED)) {
    Serial.println("Starting CAN failed! restart the program to try again");
    while (1) delay(10);
  }


  {
    Serial.print("software ID build number 0x1E : ");
    PGN_Request_Resp r = request(0x1e);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else 
      Serial.println(r.dataWord[0]);
  }
  {
    Serial.print("software ID D-Number 0x20 : ");
    PGN_Request_Resp r = request(0x20);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else 
      Serial.println(r.dataWord[0]);
  }
  {
    Serial.print("software ID version 0x22 : ");
    PGN_Request_Resp r = request(0x22);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else 
      Serial.println(r.dataWord[0]);
  }
  {
    Serial.print("unbalanced: measured value 0x2A : ");
    PGN_Request_Resp r = request(0x2a);
    if(r.data[1] == 255)
      Serial.println("SNV");
    else {
      Serial.print("%");
      Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("unbalanced: alarm threshold 0x2E : ");
    PGN_Request_Resp r = request(0x2e);
    Serial.println(r.data[1]);
  }
  {
    Serial.print("unbalanced: measured value 0x2E : ");
    PGN_Request_Resp r = request(0x2e);
    if(r.data[1] == 0)
      Serial.println("alarm deactivated");
    else {
      Serial.print("(expected %5-%45) %");
      Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("self-holding isolation alarm: activation 0x30: ");
    PGN_Request_Resp r = request(0x30);
    switch(r.data[1]){
      case 0xFC: Serial.println("false=automatic alarm reset"); break;
      case 0xFD: Serial.println("true=self-holding alarm reset"); break;
      default: 
        Serial.print("unknown value : "); 
        Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("isolation: measurement counter 0x36 : ");
    PGN_Request_Resp r = request(0x36);
    Serial.println(r.data[1]);
  }
  {
    Serial.print("isolation: active profile 0x38 : ");
    PGN_Request_Resp r = request(0x38);
    switch(r.data[1]){
      case 0: Serial.println("custom profile"); break;
      case 1: Serial.println("standard with fast startup"); break;
      case 2: Serial.println("standard"); break;
      case 3: Serial.println("high capacity with fast startup"); break;
      case 4: Serial.println("high capacity"); break;
      case 5: Serial.println("disturbed"); break;
      case 6: Serial.println("service"); break;
      case 7: Serial.println("UG"); break;
      default: 
        Serial.print("unexpected value: "); 
        Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("isolation: quality 0x3E : ");
    PGN_Request_Resp r = request(0x3e);
    if(r.data[1] == 255)
      Serial.println("SNV");
    else {
      Serial.print("%");
      Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("isolation: R_iso_neg 0x40: ");
    PGN_Request_Resp r = request(0x40);
    if(r.dataWord[1] == 65535)
      Serial.println("SNV");
    else {
      Serial.print(r.dataWord[1]);
      Serial.println(" K ohm");
    }
  }
  {
    Serial.print("isolation: R_iso_pos 0x42: ");
    PGN_Request_Resp r = request(0x42);
    if(r.dataWord[1] == 65535)
      Serial.println("SNV");
    else {
      Serial.print(r.dataWord[1]);
      Serial.println(" K ohm");
    }
  }
  {
    Serial.print("R_ios_status 0x44 : ");
    PGN_Request_Resp r = request(0x44);
    switch(r.data[1]){
      case 0xFC: Serial.println("estimated isolation value during startup"); break;
      case 0xFD: Serial.println("first measured isolation value during startup"); break;
      case 0xFE: Serial.println("isolation value in normal operation"); break;
      case 0xFF: Serial.println("SNV"); break;
      default: 
        Serial.print("unknown value : "); 
        Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("isolation: threshold error 0x46 (expected 30-2000) : ");
    PGN_Request_Resp r = request(0x46);
    Serial.println(r.dataWord[1]);
  }
  {
    Serial.print("isolation: threhold timeout measurement 0x48 (expeced 1-64255): ");
    PGN_Request_Resp r = request(0x48);
    if(r.dataWord[0] == 0)
      Serial.println("alarm deactivated");
    else{
      Serial.print(r.dataWord[0]);
      Serial.println(" seconds");
    }
  }
  {
    Serial.print("isolation: threshold warning 0x4A (expected 30-2000): ");
    PGN_Request_Resp r = request(0x4a);
    if(r.dataWord[1] < 30 || r.dataWord[1] > 2000){
      Serial.println(r.dataWord[1]);
    } else {
      Serial.print(r.dataWord[1]);
      Serial.println(" K ohm");
    }
  }
  {
    Serial.print("isolation: R_iso_corrected (expeced 0-35000) 0x4C : ");
    PGN_Request_Resp r = request(0x4c);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else{
      Serial.print(r.dataWord[0]);
      Serial.println(" K ohm");
    }
  }
  {
    Serial.print("isolation: time elapsed since last measurement 0x50 : ");
    PGN_Request_Resp r = request(0x50);
    Serial.print(r.dataWord[0]);
    Serial.println(" seconds");
  }
  {
    Serial.print("capacity: measured value  0x52 (expected 1-200) : ");
    PGN_Request_Resp r = request(0x52);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else if(r.dataWord[0] == 0 || r.dataWord[0] > 200){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 0.1f);
      Serial.println(" uF");
    }
  }
  {
    Serial.print("capacity: measurement counter 0x54 : ");
    PGN_Request_Resp r = request(0x54);
    Serial.println(r.data[0]);
  }
  {
    Serial.print("self test: period 0x58 : ");
    PGN_Request_Resp r = request(0x58);
    if(r.dataWord[0] == 0)
      Serial.println("automatic self test deactivated");
    else if(r.dataWord[0] > 64255){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 10);
      Serial.println(" seconds");
    }
  }
  {
    Serial.print("voltage: frequency 0x5A : ");
    PGN_Request_Resp r = request(0x5a);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else if(r.dataWord[0] > 5000){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 0.1f);
      Serial.println(" Hz");
    }
  }
  {
    Serial.print("voltage: measurement counter 0x5C : ");
    PGN_Request_Resp r = request(0x5c);
    Serial.println(r.data[0]);
  }
  {
    Serial.print("voltage: HV system 0x5E : ");
    PGN_Request_Resp r = request(0x5e);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else if(r.dataWord[0] > 64255){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 0.05f);
      Serial.println(" V");
    }
  }
  {
    Serial.print("voltage: HV_neg to earth 0x60 : ");
    PGN_Request_Resp r = request(0x60);
    if(r.dataWord[0] == 65535)
      Serial.println("SNV");
    else if(r.dataWord[0] > 64255){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 0.05f);
      Serial.println(" V");
    }
  }
  {
    Serial.print("voltage: HV_pos to earth 0x62 : ");
    PGN_Request_Resp r = request(0x62);
    if(r.dataWord[0] > 64255){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 0.05f);
      Serial.println(" V");
    }
  }
  {
    Serial.print("voltage: mode 0x64 : ");
    PGN_Request_Resp r = request(0x64);
    // Serial.println(r.raw, HEX);
    // uint32_t v = *reinterpret_cast<uint32_t*>(&r.data[1]);
    uint8_t v = r.data[1];
    switch(v){
      case 0xFC: Serial.println("AC + DC"); break;
      case 0xFD: Serial.println("AC"); break;
      case 0xFE: Serial.println("DC"); break;
      default: 
        Serial.print("unknown value : 0x"); 
        Serial.println(v, HEX);
    }
  }
  {
    Serial.print("voltage: threshold under voltage 0x66 : ");
    PGN_Request_Resp r = request(0x66);
    if(r.dataWord[0] == 0)
      Serial.println("deactive");
    else if(r.dataWord[0] > 1000){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0]);
      Serial.println(" V");
    }
  }
  {
    Serial.print("voltage: device activity 0x68 : ");
    PGN_Request_Resp r = request(0x68);
    switch(r.data[1]){
      case 0: Serial.println("initialization"); break;
      case 1: Serial.println("normal operation"); break;
      case 2: Serial.println("self test"); break;
      default: 
        Serial.print("unknown value : "); 
        Serial.println(r.data[1]);
    }
  }
  {
    Serial.print("status: lock 0x6A : ");
    PGN_Request_Resp r = request(0x6A);
    // uint32_t v = *reinterpret_cast<uint32_t*>(&r.data[1]);
    uint8_t v = r.data[1];
    switch(v){
      case 0xFC: Serial.println("parameter write enabled"); break;
      case 0xFD: Serial.println("parameter write disabled"); break;
      default: 
        Serial.print("unknown value : 0x"); 
        Serial.println(v, HEX);
    }
  }
  {
    Serial.println("status: warnings and alarms 0x6C : ");
    PGN_Request_Resp r = request(0x6c);
    uint32_t v = *reinterpret_cast<uint32_t*>(&r.data[1]);

    if(v & (1 << 0))
      Serial.println("    device error active");
    if(v & (1 << 1))
      Serial.println("    HV_pos connection failure");
    if(v & (1 << 2))
      Serial.println("    HV_neg connection failure");
    if(v & (1 << 3))
      Serial.println("    Earth connection failure");
    if(v & (1 << 4))
      Serial.println("    Iso alarm (iso value bellow thresh error)");
    if(v & (1 << 5))
      Serial.println("    Iso warning (iso value bellow thresh warn)");
    if(v & (1 << 6))
      Serial.println("    Iso outdated (value „Time elapsed since lst measurement“ > = „measurement timeout“)");
    if(v & (1 << 6))
      Serial.println("    Unbalance Alarm (unbalance value below threshold)");
    if(v & (1 << 8))
      Serial.println("    Undervoltage alarm");
    if(v & (1 << 9))
      Serial.println("    Unsafe to start");
    if(v & (1 << 10))
      Serial.println("    Earthlift open");
    if(v == 0)
      Serial.println("    No warnings or alarms");
  }
  {
    Serial.print("Earthlift: status 0x70 : ");
    PGN_Request_Resp r = request(0x70);
    switch(r.dataWord[0]){
      case 0xFC: Serial.println("earth disconnector closed"); break;
      case 0xFD: Serial.println("earth disconnector open"); break;
      default: 
        Serial.print("unknown value : 0x"); 
        Serial.println(r.dataWord[0], HEX);
    }
  }
  {
    Serial.print("isolation: Threshold_first_refrence_extimation 0x72 : ");
    PGN_Request_Resp r = request(0x72);
    if(r.dataWord[0] > 1000 || r.dataWord[0] == 0){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0]);
      Serial.println(" V");
    }
  }
  {
    Serial.print("isolation: Pre_estimation_max_difference 0x74 : ");
    PGN_Request_Resp r = request(0x74);
    if(r.dataWord[0] > 64255){
      Serial.print("unexpected value ");
      Serial.println(r.dataWord[0]);
    } else {
      Serial.print(r.dataWord[0] * 0.01f);
      Serial.println(" V");
    }
  }
}

void loop() {
}

PGN_Request_Resp request(uint8_t idx) {
  CAN.beginExtendedPacket(0x18EFF417, 1, false);
  CAN.write(idx);
  CAN.endPacket();

  PGN_Request_Resp ret;

  while(true) {
    int packetSize = CAN.parsePacket();

    if (packetSize == 8                   // general filtering
        && CAN.packetExtended()           // ^ 
        && !CAN.packetRtr()               // ^
        && CAN.packetId() == 0x18EF17F4)  // is response to a request?
    {

      for(int i = 0; i < 8 && CAN.available(); i++)
        ret.data[i] = CAN.read();

      // if response to the correct request
      if(ret.index == idx)
        return ret;
    }
    yield();
    delay(20);
  }
};
