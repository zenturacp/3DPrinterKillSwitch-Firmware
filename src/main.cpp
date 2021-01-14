#include <Arduino.h>
#include <EEPROM.h>
#include "header.h"

#define addr_bedMax 0
#define addr_nozzleMax 2
#define addr_delayMax 4

int currentPage = 0;

int delayMax = 60;

int bedCurrent = 21;
int bedMax;

int nozzleCurrent = 20;
int nozzleMax;

void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
  EEPROM.commit();
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void nextPage() {
  if (currentPage >= 3){
    drawPage(0);
  } else {
    drawPage(currentPage + 1);
  }
}

void drawPage(int pageNumber)
{
  //Serial.println((String)currentPage + " - " + pageNumber);
  if (pageNumber != currentPage)
  {
    switch (pageNumber)
    {
    case 0:
      Serial.println("======== Dashboard =========");
      Serial.println((String) "Bed    Status: " + bedCurrent + "/" + bedMax);
      Serial.println((String) "Nozzle Status: " + nozzleCurrent + "/" + nozzleMax);
      break;
    case 1:
      Serial.println("======== Setup Nozzle max Temperature ==========");
      Serial.println((String)"Current Setting is " + nozzleMax + ", press set to increment");
      break;
    case 2:
      Serial.println("======== Setup Bed max Temperature ==========");
      Serial.println((String)"Current Setting is " + bedMax + ", press set to increment");
      break;
    case 3:
      Serial.println("======== Setup Delay Temperature ==========");
      Serial.println((String)"Current Setting is " + delayMax + ", press set to increment");
      break;
    }
    currentPage = pageNumber;
  }
}

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(512);
  delay(1000);

  // writeIntIntoEEPROM(nozzleMaxAddress, 220);
  // writeIntIntoEEPROM(bedMaxAddress, 70);
  // writeIntIntoEEPROM(delayMaxAddress, 60);

  nozzleMax = readIntFromEEPROM(addr_nozzleMax);
  bedMax = readIntFromEEPROM(addr_bedMax);
  delayMax = readIntFromEEPROM(addr_delayMax);

  Serial.println("Startup");
  Serial.println((String) "Bed    Status: " + bedCurrent + "/" + bedMax);
  Serial.println((String) "Nozzle Status: " + nozzleCurrent + "/" + nozzleMax);
}

void loop()
{
  nextPage();
  delay(1000);
}