#ifndef DS1307_SIGFOX_EEPROM_RCT_DHT
#define DS1307_SIGFOX_EEPROM_RCT_DHT
#include <ArduinoLowPower.h>
#include <SigFox.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "conversions.h"
#include "DHT_Temp.h"
#include "EEPROM24T256.h"
volatile int alarm_source = 0;  //variable for sleep mode
SigfoxMessage_message Store_to_EEProm = {0};
RTC_Time_Typedef Rtc_mo;


extern DHT dht;
float Temperature = 0.0;

void setup ()
{
  Wire.begin();
  //Rtc_Init();
  dht.begin();
  Serial.begin(115200);
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);
  SigFox.begin();
  if (!SigFox.begin()) 
  {
    Serial.println("Something is wrong, try rebooting device");
    //SigFox.reset();
    while (1);
  }
}

void loop ()
{
  SigfoxMessage_message Read_From_EEProm = {0};
  Store_to_EEProm.Temperature = Read_Temp();
  Serial.print("Read Temperature:");
  Serial.println(Store_to_EEProm.Temperature);
  writeDataToEEPROM(DATA_ADDRESS, Store_to_EEProm);
  Read_From_EEProm = readDataFromEEPROM(DATA_ADDRESS);
  Serial.print("Temperature from EEPROM:");
  Serial.println(Read_From_EEProm.Temperature);
  Serial.print("Bytes To Be Sent:");
  Serial.println(sizeof(Store_to_EEProm));
  delay(1000);
  SigFox.begin();
  SigFox.beginPacket();
  SigFox.write((uint8_t*)&Read_From_EEProm, sizeof(Read_From_EEProm));
  int ret = SigFox.endPacket();
  Serial.println("Ret:");
  Serial.println(ret);
  delay(10000);
}
void alarmEvent0() // Interrupt to wake-up Device from SLEEP
{
  alarm_source = 0;
}
#endif
