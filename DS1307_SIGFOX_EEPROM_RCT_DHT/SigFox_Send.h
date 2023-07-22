#include <stdint.h>
#include "RTC_Time.h"

typedef struct __attribute__ ((packed)) SigfoxMessage_message 
{
    float Temperature;
    RTC_Time_Typedef Rtc_data;
} SigfoxMessage_message;
