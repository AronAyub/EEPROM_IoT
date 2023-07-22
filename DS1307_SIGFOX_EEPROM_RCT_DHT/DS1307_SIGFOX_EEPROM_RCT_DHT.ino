#include <ArduinoLowPower.h>
#include <SigFox.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "conversions.h"
#include "DHT_Temp.h"
#include "EEPROM24T256.h"

SigfoxMessage_message Store_to_EEProm={0};
SigfoxMessage_message Read_From_EEProm={0};

RTC_Time_Typedef Rtc_mo;


extern DHT dht;
float Temperature=0.0;

void setup () 
{
  Rtc_Init();
  dht.begin();
  Serial.begin(9600);
}

void loop () 
{

  Store_to_EEProm.Temperature=Read_Temp();
  Read_Rtc(&Rtc_mo);
  Store_to_EEProm.Rtc_data.T_Hrs=Rtc_mo.T_Hrs;
  Store_to_EEProm.Rtc_data.T_min=Rtc_mo.T_min;
  Store_to_EEProm.Rtc_data.T_Sec=Rtc_mo.T_Sec;
  Store_to_EEProm.Rtc_data.Date=Rtc_mo.Date;
  Store_to_EEProm.Rtc_data.Day=Rtc_mo.Day;
  Store_to_EEProm.Rtc_data.Month=Rtc_mo.Month;
  Store_to_EEProm.Rtc_data.Year=Rtc_mo.Year;
  writeDataToEEPROM(DATA_ADDRESS,&Store_to_EEProm);
  Read_From_EEProm = readDataFromEEPROM(DATA_ADDRESS);
  Serial.println("Temperature from EEPROM:");
  Serial.print(Read_From_EEProm.Temperature);
  Serial.print("\n");
  delay(1000);
}
