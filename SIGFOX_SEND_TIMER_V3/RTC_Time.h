#ifndef TC_TIME_
#define TC_TIME_
#include "RTClib.h"
#include <stdint.h>
#include "SigFox_Send.h"

RTC_DS1307 rtc;

typedef struct RTC_Time_Typedef
{
 
  uint8_t T_min;
  uint8_t T_Sec; //
  uint8_t T_Hrs; 8
  uint8_t Date; 25
  uint8_t Day; 1 //
  uint8_t Month; 9
  uint16_t Year; 2023
}RTC_Time_Typedef;

//

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
