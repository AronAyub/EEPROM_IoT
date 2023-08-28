
#ifndef SIGFOX_SEND
#define SIGFOX_SEND
#include <stdint.h>
#include "RTC_Time.h"


typedef struct __attribute__ ((packed)) SigfoxMessage_message 
{
    float Temperature;
    RTC_Time_Typedef Rtc_data;
} SigfoxMessage_message;


void Pack_data_to_store(RTC_Time_Typedef* Rtc_time,float temp,SigfoxMessage_message *data)
{
  data->Temperature=temp;
  data->Rtc_data.T_min=Rtc_time->T_min;
  data->Rtc_data.T_Hrs=Rtc_time->T_Hrs;
  data->Rtc_data.Date=Rtc_time->Date;
}

#endif
