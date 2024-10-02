#include <SoftwareSerial.h>
#include <RadioLib.h>
#include "boards.h"
#include "images.h"
#include <Arduino_JSON.h>
#include <TinyGPS.h>

//**********************************************
//*  Define the ID (Name) of your Module here  *
#define MODULE_NAME "Transmitter1A"

SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DI0_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);

//GPS data
TinyGPS gps;
SoftwareSerial ss(13, 34);
String jsonString;
int state;


static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

void setup() {
  ss.begin(9600);

  initBoard();
  u8g2->clearBuffer();
  u8g2->drawXBMP(0, 0, OLu_Engineering_width, OLu_Engineering_height, OLu_Engineering_bits);
 
  u8g2->sendBuffer();
  // When the power is turned on, a delay is required.
  delay(3000);

}


void loop() {
   // initialize SX1276 with default settings
  Serial.print(F("[SX1276] Initializing ... "));
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
    while (true)
      ;
  }
//make packet
  float flat, flon;
    unsigned long age, date, time, chars = 0;
    unsigned short sentences = 0, failed = 0;
    static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
 

    print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
    print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
    gps.f_get_position(&flat, &flon, &age);
    print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 10, 6);
    print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 11, 6);
    print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
    print_date(gps);

    /*if (flat != TinyGPS::GPS_INVALID_F_ANGLE && flon != TinyGPS::GPS_INVALID_F_ANGLE) {
    jsonString = make_JSON(flat, flon, gps);
    Serial.println(F("JString..."));
    Serial.println(jsonString);
    state = radio.transmit(jsonString);
    }*/

    print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 7, 2);
    print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
    print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
    print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
   print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0xFFFFFFFF : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
    print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? TinyGPS::GPS_INVALID_F_ANGLE : TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
    print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

    gps.stats(&chars, &sentences, &failed);
    print_int(chars, 0xFFFFFFFF, 6);
    print_int(sentences, 0xFFFFFFFF, 10);
    print_int(failed, 0xFFFFFFFF, 9);
    Serial.println();

    jsonString = make_JSON(flat, flon, gps);
    Serial.println(F("JString..."));
    Serial.println(jsonString);
    state = radio.transmit(jsonString);

    smartdelay(1000);
//finish making packet

  Serial.print(F("[SX1276] Transmitting packet ... "));


  if (state == RADIOLIB_ERR_NONE) {
    // the packet was successfully transmitted
    Serial.println(F(" success!"));

    // print measured data rate
    Serial.print(F("[SX1276] Datarate:\t"));
    Serial.print(radio.getDataRate());
    Serial.println(F(" bps"));

    char buf[256];
    u8g2->clearBuffer();
    u8g2->drawStr(0, 12, "Transmitting: OK!");
    snprintf(buf, sizeof(buf), "Rate: %.2f bps", radio.getDataRate());
    u8g2->drawStr(5, 30, buf);
    u8g2->sendBuffer();

  } else if (state == RADIOLIB_ERR_PACKET_TOO_LONG) {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));

  } else if (state == RADIOLIB_ERR_TX_TIMEOUT) {
    // timeout occurred while transmitting packet
    Serial.println(F("timeout!"));

  } else {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
  delay(5000);

 /* Serial.println("Going to sleep...");
  u8g2->drawStr(5, 48, "Going to Sleep");
  u8g2->sendBuffer();
  delay(2000);

  //**********************************************
  //*  This the sleep funtion that will put your *
  //*  chip into ultra-low power mode for the    *
  //*  number of seconds that you specify.  It   *
  //*  will then reboot and run setup() again.   *
  //**********************************************
  goToSleep(5);*/
}

String make_JSON(float flat, float flong, TinyGPS &gps) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  JSONVar myObject;

  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  myObject["ID"] = MODULE_NAME;  // Do not change here - change in definition at top of program
  myObject["Latitude"] = flat;
  myObject["Longitude"] = flong;
  myObject["Hours"] = hour;
  myObject["Minutes"] = minute;
  myObject["Seconds"] = second;
  myObject["Month"] = month;
  myObject["Day"] = day;
  myObject["Year"] = year;

  // This line makes all of your data into a single String that can be transmitted
  String s = JSON.stringify(myObject);
  return s;
}

void goToSleep(int sec) {
  u8g2->sleepOn();
  radio.sleep();

  SPI.end();
  SDSPI.end();

  pinMode(RADIO_CS_PIN, INPUT);
  pinMode(RADIO_RST_PIN, INPUT);
  pinMode(RADIO_DI0_PIN, INPUT);
  pinMode(RADIO_CS_PIN, INPUT);
  pinMode(I2C_SDA, INPUT);
  pinMode(I2C_SDA, INPUT);
  pinMode(I2C_SCL, INPUT);
  pinMode(OLED_RST, INPUT);
  pinMode(RADIO_SCLK_PIN, INPUT);
  pinMode(RADIO_MISO_PIN, INPUT);
  pinMode(RADIO_MOSI_PIN, INPUT);
  pinMode(BOARD_LED, INPUT);
  pinMode(ADC_PIN, INPUT);

  randomSeed(analogRead(0));
  delay(2000 + random(0, 1000));

  esp_sleep_enable_timer_wakeup(sec * 1000 * 1000);
  esp_deep_sleep_start();
}

//more GPS methods
static void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void print_float(float val, float invalid, int len, int prec) {
  if (val == invalid) {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  } else {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);  // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3
                           : vi >= 10  ? 2
                                       : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartdelay(0);
}

static void print_int(unsigned long val, unsigned long invalid, int len) {
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartdelay(0);
}

static void print_date(TinyGPS &gps) {
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  hour -= 7;
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("********** ******** ");
  else {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
            month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  smartdelay(0);
}

static void print_str(const char *str, int len) {
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartdelay(0);
}