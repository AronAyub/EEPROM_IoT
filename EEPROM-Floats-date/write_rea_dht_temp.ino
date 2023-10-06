#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define EEPROM_ADDRESS 0x50
#define DHT_PIN 2
#define DHT_TYPE DHT22 // Change this to DHT11, DHT21, or DHT22 depending on your sensor

DHT dht(DHT_PIN, DHT_TYPE);

struct TemperatureReading {
  int address;
  float temperature;
};

int numReadings = 0; // Track the number of readings

void setup() {
  Serial.begin(9600);
  Wire.begin();
  dht.begin();

  while (!Serial) {
    // Wait for the serial monitor to open
  }
}

void writeFloatToEEPROM(int address, float value) {
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.write(p[i]); // Write each byte of the float
    Wire.endTransmission();
    delay(5); // Delay to ensure writes are complete
  }
}

float readFloatFromEEPROM(int address) {
  float value;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((address + i) >> 8); // High byte of address
    Wire.write((address + i) & 0xFF); // Low byte of address
    Wire.endTransmission();

    // Read data from the EEPROM
    Wire.requestFrom(EEPROM_ADDRESS, 1); // Read one byte at a time

    if (Wire.available()) {
      p[i] = Wire.read(); // Read each byte of the float
    }
  }
  return value;
}

void loop() {
  float temperature = dht.readTemperature();

  // Calculate the EEPROM address based on the number of readings
  int eepromAddress = numReadings * sizeof(float);

  // Write the temperature value to EEPROM
  writeFloatToEEPROM(eepromAddress, temperature);

  // Create a TemperatureReading struct to store address and temperature
  TemperatureReading reading;
  reading.address = eepromAddress;
  reading.temperature = temperature;

  // Increment the number of readings
  numReadings++;

  // Print the memory address and temperature
  Serial.print("Address: ");
  Serial.print(reading.address);
  Serial.print(", Temperature: ");
  Serial.println(reading.temperature);

  delay(10000); // Wait for 10 seconds before the next reading
}
