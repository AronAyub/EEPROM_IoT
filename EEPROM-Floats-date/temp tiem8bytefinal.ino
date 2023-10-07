#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <RTClib.h>

#define EEPROM_ADDRESS 0x50
#define DHT_PIN 2
#define DHT_TYPE DHT22 // Change this to DHT11, DHT21, or DHT22 depending on your sensor

DHT dht(DHT_PIN, DHT_TYPE);
RTC_DS3231 rtc;

struct TemperatureReading {
  int address;
  float temperature;
  DateTime timestamp;
};

int numReadings = 0; // Track the number of readings

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
}

void writeTemperatureToEEPROM(int address, float temperature, DateTime timestamp) {
  byte* p = (byte*)(void*)&temperature;
  for (int i = 0; i < sizeof(temperature); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.write(p[i]); // Write each byte of the float
    Wire.endTransmission();
    delay(5); // Delay to ensure writes are complete
  }
  
  // Write the timestamp as Unix time (4 bytes) to EEPROM
  unsigned long unixTime = timestamp.unixtime();
  p = (byte*)(void*)&unixTime;
  for (int i = 0; i < sizeof(unixTime); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + sizeof(temperature) + i) >> 8); // High byte of address
    Wire.write((address + sizeof(temperature) + i) & 0xFF); // Low byte of address
    Wire.write(p[i]); // Write each byte of the Unix time
    Wire.endTransmission();
    delay(5); // Delay to ensure writes are complete
  }
}

void readTemperatureFromEEPROM(int address, TemperatureReading& reading) {
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
  Serial.print(reading.timestamp.second(), DEC);
  Serial.print(", Data Size (bytes): ");
  int dataSize = sizeof(float) + sizeof(unsigned long);
  Serial.println(dataSize);
}

void loop() {
  float temperature = dht.readTemperature();
  DateTime now = rtc.now();
  TemperatureReading reading;
  reading.address = numReadings * (sizeof(float) + sizeof(unsigned long)); // Calculate address
  reading.temperature = temperature;
  reading.timestamp = now;

  writeTemperatureToEEPROM(reading.address, reading.temperature, reading.timestamp);

  // Increment the number of readings
  numReadings++;

  // Print the memory address, temperature, and timestamp
  printTemperatureReading(reading);

  delay(10000);
}
