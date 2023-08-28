#ifndef DS1307_SIGFOX_EEPROM_RCT_DHT
#define DS1307_SIGFOX_EEPROM_RCT_DHT
#include <Arduino.h>
#include "Adafruit_ZeroTimer.h"
#include <ArduinoLowPower.h>
#include <SigFox.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "conversions.h"
#include "DHT_Temp.h"
#include "EEPROM24T256.h"
volatile int alarm_source = 0;  //variable for sleep mode
SigfoxMessage_message Store_to_EEProm = {0};
SigfoxMessage_message Read_From_EEProm = {0};
RTC_Time_Typedef Rtc_mo;


volatile int Timer_seconds=0;
volatile int Read_store=0;
extern DHT dht;
float Temperature = 0.0;
RTC_Time_Typedef sTime = {0};
float freq = 1; // 1 KHz
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

volatile bool togglepin = false;
void TimerCallback0(void)
{
  if (Timer_seconds < 15)
  {
    Timer_seconds++;
  }
  else
  {
    Read_store=1;
    Timer_seconds = 0;
  }
}

void setup ()
{
  Wire.begin();
  Rtc_Init();
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
  // Set up the flexible divider/compare
  uint8_t divider  = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

  if ((freq < 24000000) && (freq > 800)) {
    divider = 1;
    prescaler = TC_CLOCK_PRESCALER_DIV1;
    compare = 48000000 / freq;
  } else if (freq > 400) {
    divider = 2;
    prescaler = TC_CLOCK_PRESCALER_DIV2;
    compare = (48000000 / 2) / freq;
  } else if (freq > 200) {
    divider = 4;
    prescaler = TC_CLOCK_PRESCALER_DIV4;
    compare = (48000000 / 4) / freq;
  } else if (freq > 100) {
    divider = 8;
    prescaler = TC_CLOCK_PRESCALER_DIV8;
    compare = (48000000 / 8) / freq;
  } else if (freq > 50) {
    divider = 16;
    prescaler = TC_CLOCK_PRESCALER_DIV16;
    compare = (48000000 / 16) / freq;
  } else if (freq > 12) {
    divider = 64;
    prescaler = TC_CLOCK_PRESCALER_DIV64;
    compare = (48000000 / 64) / freq;
  } else if (freq > 3) {
    divider = 256;
    prescaler = TC_CLOCK_PRESCALER_DIV256;
    compare = (48000000 / 256) / freq;
  }
  else if (freq >= 0.75)
  {
    divider = 1024;
    prescaler = TC_CLOCK_PRESCALER_DIV1024;
    compare = (48000000 / 1024) / freq;
  }
  else
  {
    Serial.println("Invalid frequency");
    while (1) delay(10);
  }
  Serial.print("Divider:"); Serial.println(divider);
  Serial.print("Compare:"); Serial.println(compare);
  Serial.print("Final freq:"); Serial.println((int)(48000000 / compare));

  zerotimer.enable(false);
  zerotimer.configure(prescaler,       // prescaler
                      TC_COUNTER_SIZE_16BIT,       // bit width of timer/counter
                      TC_WAVE_GENERATION_MATCH_PWM // frequency or PWM mode
                     );

  zerotimer.setCompare(0, compare);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, TimerCallback0);
  zerotimer.enable(true);
}


void loop ()
{
  if (Read_store==1)
  {
  Temperature = Read_Temp();
  Read_Rtc(&sTime);
  Pack_data_to_store(&sTime, Temperature, &Store_to_EEProm);
  Serial.print("Read Temperature:");
  Serial.println(Store_to_EEProm.Temperature);
  writeDataToEEPROM(DATA_ADDRESS, Store_to_EEProm);
  Read_From_EEProm = readDataFromEEPROM(DATA_ADDRESS);
  Serial.print("Temperature from EEPROM:");
  Serial.println(Read_From_EEProm.Temperature);

  Serial.print("Min:");
  Serial.println(Read_From_EEProm.Rtc_data.T_min);
  Serial.print("Hours:");
  Serial.println(Read_From_EEProm.Rtc_data.T_Hrs);
  Serial.print("Date:");
  Serial.println(Read_From_EEProm.Rtc_data.Date);
  Serial.print("Bytes To Be Sent:");
  Serial.println(sizeof(Store_to_EEProm));
  Read_store=0;
//  SigFox.begin();
//  SigFox.beginPacket();
//  SigFox.write((uint8_t*)&Read_From_EEProm, sizeof(Read_From_EEProm));
//  int ret = SigFox.endPacket();
//  Serial.println("Ret:");
//  Serial.println(ret);
  }
  else
  {
    Serial.print("TIMER:"); 
    Serial.print(Timer_seconds);
    Serial.println(" S");
    delay(1000);
    }
}

void alarmEvent0() // Interrupt to wake-up Device from SLEEP
{
  alarm_source = 0;
}
#endif
