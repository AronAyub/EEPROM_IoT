CODE: SELECT ALL

/*
  External EEPROM Recording
  //Uses AT24LC256 External I2C EEPROM
  DroneBot Workshop 2019

  Uses AT24C256 EEPROM Memory Module 8P Chip 
  Carrier Power Supply Indicator
https://www.cafr.ebay.ca/itm/4Pcs-AT24C256-EEPROM-Memory-Module-8P-Chip-Carrier-Power-Supply-Indicator/162937327097?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649
*/

// Include the I2C Wire Library
#include "Wire.h"

// EEPROM I2C Address
#define EEPROM_I2C_START_ADDRESS 0x7B0C  //31500

// Integer to hold number of addresses to fill
int maxaddress = 32100; //32K  bytes?

// Byte to hold data read from EEPROM
int readVal = 0;

//-----------------------------
void setup()
{
  // Connect to I2C bus as master
  Wire.begin();

  // Setup Serial Monitor
  Serial.begin(9600);

  writEr();

  readEr();
}
// Function to write to EEPROOM
void writeEEPROM(int address, byte val, int i2c_address)
{
  // Begin transmission to I2C EEPROM
  Wire.beginTransmission(i2c_address);

  // Send memory address as two 8-bit bytes
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB

  // Send data to be stored
  Wire.write(val);

  // End the transmission
  Wire.endTransmission();

  // Add 5ms delay for EEPROM
  delay(5);
}

// Function to read from EEPROM
byte readEEPROM(int address, int i2c_address)
{
  // Define byte for received data
  byte rcvData = 0xFF;

  // Begin transmission to I2C EEPROM
  Wire.beginTransmission(i2c_address);

  // Send memory address as two 8-bit bytes
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB

  // End the transmission
  Wire.endTransmission();

  // Request one byte of data at current memory address
  Wire.requestFrom(i2c_address, 1);

  // Read the data and assign to variable
  rcvData =  Wire.read();

  // Return the data as function output
  return rcvData;
}

void readEr() {
  int address = EEPROM_I2C_START_ADDRESS;
  // Run until maximum address is reached

  for (; address <= maxaddress; address++) {

    // Read value from EEPROM
    readVal = readEEPROM(address, EEPROM_I2C_START_ADDRESS);

    // Print to Serial Monitor
    Serial.print("From AT24C256 = ");
    Serial.print(address);
    Serial.print("\t");
    Serial.println(readVal);

  }

  // Print to Serial Monitor
  Serial.println("Playback Finished!");
}

void writEr() {
  int address = EEPROM_I2C_START_ADDRESS;

  // Print to Serial Monitor
  Serial.println("Start Recording...");
  // Run until maximum address is reached

  for (; address <= maxaddress; address++) {
    byte val = (address/1000);
    // Record the position in the external EEPROM
    writeEEPROM(address, val, EEPROM_I2C_START_ADDRESS);

    // Print to Serial Monitor
    Serial.print("To AT24C256 = ");
    Serial.print(address);
    Serial.print("\t");
    Serial.println((address/1000));
  }

  // Print to Serial Monitor
  Serial.println("Recording Finished!");
}

void loop()
{

}