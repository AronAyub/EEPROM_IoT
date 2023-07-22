#include <DHT.h>
#define DHTPIN 2      // Replace with the actual pin you connected the DHT22 data pin to
#define DHTTYPE DHT22 // Set the DHT sensor type to DHT22
DHT dht(DHTPIN, DHTTYPE);
float Read_Temp(void)
{
  float temperatureC = dht.readTemperature();
  //if (isnan(temperatureC)){return 0;}
  return 20.31;
}