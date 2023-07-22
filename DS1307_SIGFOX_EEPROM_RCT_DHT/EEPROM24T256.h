#include <Wire.h>
#include <DHT.h>
#include "SigFox_Send.h"

// Address of the EEPROM chip (7-bit address)
const int EEPROM_ADDRESS = 0x50; // Change this address based on your EEPROM chip
// Address to store the data structure in EEPROM

const int DATA_ADDRESS = 0; // The structure will be stored starting from this address


// Function to write the data structure to EEPROM
void writeDataToEEPROM(int address,SigfoxMessage_message* Sigfoxdata) 
{
  const byte* p = (const byte*)(const void*)Sigfoxdata;
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write(address >> 8); // MSB of address
  Wire.write(address & 0xFF); // LSB of address
  for (int i = 0; i < sizeof(Sigfoxdata); i++) {
    Wire.write(*p++); // Write each byte of the data structure
  }
  Wire.endTransmission();
  delay(5); // Small delay to ensure the data is written properly
}


// Function to read the data structure from EEPROM
SigfoxMessage_message readDataFromEEPROM(int address) 
{
  SigfoxMessage_message data;
  byte* p = (byte*)(void*)&data;
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write(address >> 8); // MSB of address
  Wire.write(address & 0xFF); // LSB of address
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, sizeof(SigfoxMessage_message));
  for (int i = 0; i < sizeof(SigfoxMessage_message); i++) {
    if (Wire.available()) {
      *p++ = Wire.read(); // Read each byte into the data structure
    }
  }
  return data;
}