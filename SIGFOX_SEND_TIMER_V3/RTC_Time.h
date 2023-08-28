#ifndef TC_TIME_
#define TC_TIME_
#include "RTClib.h"
#include <stdint.h>
#include "SigFox_Send.h"

RTC_DS1307 rtc;

typedef struct RTC_Time_Typedef
{
  uint8_t T_Hrs;
  uint8_t T_min;
  uint8_t T_Sec;
  uint8_t Date;
  uint8_t Day;
  uint8_t Month;
  uint16_t Year;
}RTC_Time_Typedef;


void Rtc_Init(void)
{
  if (! rtc.begin()) 
  {
    while (1) delay(10);
  }
  if (! rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}
void Read_Rtc(RTC_Time_Typedef* Rtc_time)
{
  DateTime now = rtc.now();
  Rtc_time->Year = now.year();
  Rtc_time->Month= now.month();
  Rtc_time ->Date= now.day();
  Rtc_time->Day= now.dayOfTheWeek();
  Rtc_time->T_Hrs = now.hour();
  Rtc_time->T_min= now.minute();
  Rtc_time->T_Sec = now.second();
}

#endif
