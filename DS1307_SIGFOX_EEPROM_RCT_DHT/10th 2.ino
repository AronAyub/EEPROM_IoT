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
volatile int Timer_seconds = 0;
volatile int Read_store = 0;
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

void TimerCallback0(void) {
  if (Timer_seconds < 10) {
    Timer_seconds++;
  } else {
    if (recordingData) {
      // Record temperature and timestamp data
      float temperature = dht.readTemperature();
      DateTime now = rtc.now();
      TemperatureReading reading;
      reading.address = numReadings * (sizeof(float) + sizeof(unsigned long)); // Calculate address
      reading.temperature = temperature;
      reading.timestamp = now;
      writeTemperatureToEEPROM(reading.address, reading.temperature, reading.timestamp);
      numReadings++;

      if (numReadings >= 60) { // 3 days of data recorded (30 seconds * 4320 = 4320 minutes)
        recordingData = false; // Stop recording data after 3 days
      }
    } else {
      // Resume reading at the same interval
      Read_store = 1;
      Timer_seconds = 0;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();

  if (!rtc.begin()) {
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
  byte* p = (byte*)(void*)&temperature;
  for (int i = 0; i < sizeof(temperature); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.write(p[i]);
    Wire.endTransmission();
    delay(5);
  }

  unsigned long unixTime = timestamp.unixtime();

  p = (byte*)(void*)&unixTime;
  for (int i = 0; i < sizeof(unixTime); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + sizeof(temperature) + i) >> 8); // High byte of address
    Wire.write((address + sizeof(temperature) + i) & 0xFF); // Low byte of address
    Wire.write(p[i]);
    Wire.endTransmission();
    delay(5);
  }
}

void readTemperatureFromEEPROM(int address, TemperatureReading& reading) {
  byte bytes[sizeof(float)];
  for (int i = 0; i < sizeof(float); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.endTransmission();

    Wire.requestFrom(EEPROM_ADDRESS, 1); // Read one byte at a time

    if (Wire.available()) {
      bytes[i] = Wire.read();
    }
  }

  reading.temperature = *(float*)(void*)bytes;

  unsigned long unixTime = 0;
  byte* timeBytes = (byte*)(void*)&unixTime;
  for (int i = 0; i < sizeof(unixTime); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + sizeof(float) + i) >> 8); // High byte of address
    Wire.write((address + sizeof(float) + i) & 0xFF); // Low byte of address
    Wire.endTransmission();

    Wire.requestFrom(EEPROM_ADDRESS, 1); // Read one byte at a time

    if (Wire.available()) {
      timeBytes[i] = Wire.read();
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
  Serial.println(reading.timestamp.unixtime());
  Serial.print("Data Size (bytes): ");
  int dataSize = sizeof(float) + sizeof(unsigned long);
  Serial.println(dataSize);
  Serial.println("Next value after 30 sec: ");
}

void loop() {
  if (Read_store == 1 && !recordingData) {
    // Read and print temperature data at the same interval as defined in the timer
    float temperature = dht.readTemperature();
    DateTime now = rtc.now();
    TemperatureReading reading;
    reading.address = numReadings * (sizeof(float) + sizeof(unsigned long));
    reading.temperature = temperature;
    reading.timestamp = now;
    printTemperatureReading(reading);
    Read_store = 0;


  } else if (recordingData) {
    // Continue recording data for 3 days
    TimerCallback0(); // Simulate timer interrupt to continue data recording

    // Print the timer value while recording data
    //Serial.print("Timer (Recording): ");
    //Serial.println(Timer_seconds);
    delay(1000); // Delay to simulate the timer interval
  }
}
