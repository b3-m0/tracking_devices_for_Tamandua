#include <SoftwareSerial.h> 
#include "boards.h"
#include <RadioLib.h>
#include <Arduino_JSON.h>
int state;


#define MODULE_NAME "Receiver1A"
SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DI0_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

void setup()
{
    Serial.begin(115200); 
    initBoard();
    Serial.println("turned on");
    // When the power is turned on, a delay is required.
    

    u8g2->clearBuffer();
    u8g2->drawStr(5, 50, MODULE_NAME);
    u8g2->sendBuffer();
    delay(1500); 

    Serial.println("Hello world");

    
    Serial.println(F("[SX1276] Initializing ... "));
    state = radio.begin(LoRa_frequency);

  if (state != RADIOLIB_ERR_NONE) {
    u8g2->clearBuffer();
    u8g2->drawStr(0, 12, "Initializing: FAIL!");
    u8g2->sendBuffer();
  }

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}


void loop() {
  Serial.print(F("[SX1278] Waiting for incoming transmission ... "));

  // you can receive data as an Arduino String
  String str;
  int state = radio.receive(str);

  // you can also receive data as byte array
  /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */

  if (state == RADIOLIB_ERR_NONE) {
    // packet was successfully received
    Serial.println(F("success!"));

    // print the data of the packet
    Serial.print(F("[SX1278] Data:\t\t\t"));
    Serial.println(str);
  }
}

 void objRec(JSONVar myObject) {
  Serial.println("{");
  for (int x = 0; x < myObject.keys().length(); x++) {
    if ((JSON.typeof(myObject[myObject.keys()[x]])).equals("object")) {
      Serial.print(myObject.keys()[x]);
      Serial.println(" : ");
      objRec(myObject[myObject.keys()[x]]);
    }
    else {
      Serial.print(myObject.keys()[x]);
      Serial.print(" : ");
      Serial.println(myObject[myObject.keys()[x]]);
    }
  }
  Serial.println("}");
}
