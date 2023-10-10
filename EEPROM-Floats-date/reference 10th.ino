#include "Adafruit_ZeroTimer.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <RTClib.h>

#define EEPROM_ADDRESS 0x50
#define DHT_PIN 2
#define DHT_TYPE DHT22 // Change this to DHT11, DHT21, or DHT22 depending on your sensor

DHT dht(DHT_PIN, DHT_TYPE);
RTC_DS3231 rtc;

volatile int alarm_source = 0;  //variable for sleep mode
volatile int Timer_seconds=0;
volatile int Read_store=0;
volatile bool recordingData = true; // New variable to control data recording
int numReadings = 0; // Track the number of readings

float freq = 1; // 1 KHz
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);


struct TemperatureReading {
  int address;
  float temperature;
  DateTime timestamp;
};

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}

volatile bool togglepin = false;
void TimerCallback0(void)
{
  if (Timer_seconds < 30)
  {
    Timer_seconds++;
  }
  else
  {
    Read_store=1;
    Timer_seconds = 0;
  }
}



void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  while (!Serial) {
    // Wait for the serial monitor to open
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


void writeTemperatureToEEPROM(int address, float temperature, DateTime timestamp) {
  byte* p = (byte*)(void*)&temperature; // pointer p uses type casting to treat tmp as series of byts, why ? EEPROM data is stored as byte
  for (int i = 0; i < sizeof(temperature); i++) {//iterate through suize of temp
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.write(p[i]); // Write each byte of the float,teh 4 bytes written at this stage
    Wire.endTransmission();
    delay(5); // Delay to ensure writes are complete
  }
  
  // Write the timestamp as Unix time (4 bytes) to EEPROM
  unsigned long unixTime = timestamp.unixtime();
  
//  Serial.print("What we Write to EEPROM:"); // what we write to EEPROM
//  Serial.println(unixTime); // see what is written
//  Serial.println(temperature); // see what is written
  
  
  p = (byte*)(void*)&unixTime;
  for (int i = 0; i < sizeof(unixTime); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + sizeof(temperature) + i) >> 8); // High byte of address, ensures we store time excatly after temp value
    Wire.write((address + sizeof(temperature) + i) & 0xFF); // Low byte of address
    Wire.write(p[i]); // Write each byte of the Unix time
    Wire.endTransmission();
    delay(5); // Delay to ensure writes are complete
  }
}
//read temp and timestamp
void readTemperatureFromEEPROM(int address, TemperatureReading& reading) {
  //read Temperature floating points 

  byte bytes[sizeof(float)];
  for (int i = 0; i < sizeof(float); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.endTransmission();

    // Read data from the EEPROM
    Wire.requestFrom(EEPROM_ADDRESS, 1); // Read one byte at a time

    if (Wire.available()) {
      bytes[i] = Wire.read(); // Read each byte of the float
    }
  }
  
  reading.temperature = *(float*)(void*)bytes; // Convert bytes back to float

  // Read the Unix time (4 bytes) from EEPROM and convert it to DateTime
  unsigned long unixTime = 0;
  byte* timeBytes = (byte*)(void*)&unixTime;
  for (int i = 0; i < sizeof(unixTime); i++) {
       
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + sizeof(float) + i) >> 8); // High byte of address
    Wire.write((address + sizeof(float) + i) & 0xFF); // Low byte of address
    Wire.endTransmission();

    // Read data from the EEPROM
    Wire.requestFrom(EEPROM_ADDRESS, 1); // Read one byte at a time

    if (Wire.available()) {
      timeBytes[i] = Wire.read(); // Read each byte of the Unix time
    }
  }
  reading.timestamp = DateTime(unixTime);

}

void printTemperatureReading(const TemperatureReading& reading) {
  Serial.print("Address: ");
  Serial.print(reading.address);
  Serial.print(", Temperature: ");
  Serial.print(reading.temperature);
  Serial.print(", Timestamp: ");
  Serial.print(reading.timestamp.year(), DEC);
  Serial.print('/');
  Serial.print(reading.timestamp.month(), DEC);
  Serial.print('/');
  Serial.print(reading.timestamp.day(), DEC);
  Serial.print(' ');
  Serial.print(reading.timestamp.hour(), DEC);
  Serial.print(':');
  Serial.print(reading.timestamp.minute(), DEC);
  Serial.print(':');
  Serial.println(reading.timestamp.second(), DEC);
  //Serial.println("UnixT");
  
  Serial.println(reading.timestamp.unixtime());


  Serial.print("Data Size (bytes): ");
  int dataSize = sizeof(float) + sizeof(unsigned long);
  Serial.println(dataSize);
  Serial.println("Next value afte 10 sec: ");
  
}

void loop() {
  

  if (Read_store==1)
  {
  float temperature = dht.readTemperature();
  DateTime now = rtc.now();
  TemperatureReading reading;
  reading.address = numReadings * (sizeof(float) + sizeof(unsigned long)); // Calculate address to be used based on float and time size
  reading.temperature = temperature; // assigns temp to temp field of the struct  
  reading.timestamp = now;

  writeTemperatureToEEPROM(reading.address, reading.temperature, reading.timestamp); //writting temp and timestamp at calculated addresses

  // Increment the number of readings
  numReadings++;

  // Print the memory address, temperature, and timestamp
  printTemperatureReading(reading);
  Read_store=0;
  }
  
else
  {
    Serial.print("TIMER:"); 
    Serial.print(Timer_seconds);
    Serial.println(" S");
    delay(1000);
    }
  
}
