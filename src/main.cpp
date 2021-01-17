#include <Arduino.h>
#include <EEPROM.h>
#include "header.h"

#define addr_bedMax 0
#define addr_nozzleMax 2
#define addr_delayMax 4

#define pinModify 18
#define pinMenu 19

#define saveDelay 5000
#define resetDisplayDelay 10000
#define updateDelay 3000

unsigned long lastUpdate;
unsigned long lastPress;
int lastSaveSettingsCount = 0;
int currentSettingsCount = 0;

bool btnLastMenu;
bool btnLastModify;

int currentPage = 0;

int delayMax = 60;

int bedCurrent = 21;
int bedMax;

int nozzleCurrent = 20;
int nozzleMax;

void saveSettings()
{
  Serial.println("Saving settings");
  writeIntIntoEEPROM(addr_nozzleMax, nozzleMax);
  writeIntIntoEEPROM(addr_bedMax, bedMax);
  writeIntIntoEEPROM(addr_delayMax, delayMax);
  blink();
  blink();
}

void blink()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
}

void writeIntIntoEEPROM(int address, int number)
{
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
  EEPROM.commit();
}

int incrementValue(int currentValue, int minValue, int maxValue, int increment)
{
  if ((currentValue + increment) > maxValue)
  {
    return 0;
  }
  else if(currentValue == 0){
    return minValue;
  } else  {
    return currentValue + increment;
  }
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void nextPage()
{
  if (currentPage >= 3)
  {
    drawPage(0);
  }
  else
  {
    drawPage(currentPage + 1);
  }
}

void drawPage(int pageNumber, bool force)
{
  blink();
  if (pageNumber != currentPage || force == true)
  {
    switch (pageNumber)
    {
    case 0:
      lastUpdate = millis();
      Serial.println((String) "======== Dashboard " + (pageNumber+1) + "/4 =========");
      if (bedMax > 0)
      {
        Serial.println((String) "Bed         Status: " + bedCurrent + "/" + bedMax);
      }
      else
      {
        Serial.println((String) "Bed         Status: " + bedCurrent + " - Disabled");
      }
      if (nozzleMax > 0)
      {
        Serial.println((String) "Nozzle      Status: " + nozzleCurrent + "/" + nozzleMax);
      }
      else
      {
        Serial.println((String) "Nozzle      Status: " + nozzleCurrent + " - Disabled");
      }
      if (delayMax == 1)
      {
        Serial.println((String) "Delay for shutdown: " + delayMax + " minute");
      }
      else if (delayMax > 1)
      {
        Serial.println((String) "Delay for shutdown: " + delayMax + " minutes");
      }
      else
      {
        Serial.println((String) "Autoshutdown - Disabled");
      }
      break;
    case 1:
      Serial.println((String) "======== Setup Nozzle max Temperature " + (pageNumber+1) + "/4 ==========");
      if (nozzleMax > 0)
      {
        Serial.println((String) "Current Setting is " + nozzleMax + ", press set to increment");
      }
      else
      {
        Serial.println((String) "Current Setting is disabled, press set to increment");
      }
      break;
    case 2:
      Serial.println((String) "======== Setup Bed max Temperature " + (pageNumber+1) + "/4 ==========");
      if (bedMax > 0)
      {
        Serial.println((String) "Current Setting is " + bedMax + ", press set to increment");
      }
      else
      {
        Serial.println((String) "Current Setting is disabled, press set to increment");
      }
      break;
    case 3:
      Serial.println((String) "======== Setup Delay Temperature " + (pageNumber+1) + "/4 ==========");
      if (delayMax == 1)
      {
        Serial.println((String) "Current Setting is " + delayMax + " minute, press set to increment");
      }
      else if (delayMax > 1)
      {
        Serial.println((String) "Current Setting is " + delayMax + " minutes, press set to increment");
      }
      else
      {
        Serial.println((String) "Current Setting is disabled, press set to increment");
      }
      break;
    }
    currentPage = pageNumber;
  }
}

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(512);
  delay(500);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pinMenu, INPUT_PULLUP);
  pinMode(pinModify, INPUT_PULLUP);

  btnLastMenu = digitalRead(pinMenu);
  btnLastModify = digitalRead(pinModify);

  nozzleMax = readIntFromEEPROM(addr_nozzleMax);
  bedMax = readIntFromEEPROM(addr_bedMax);
  delayMax = readIntFromEEPROM(addr_delayMax);

  Serial.println("==== Startup ====");
  drawPage(0, true);
  blink();
  blink();
}

void loop()
{
  bool btnNowMenu = digitalRead(pinMenu);
  bool btnNowModify = digitalRead(pinModify);

  // Save Settings of modified
  if (currentSettingsCount > lastSaveSettingsCount && (millis()-lastPress) > saveDelay){
    saveSettings();
    lastSaveSettingsCount = currentSettingsCount;
  }

  // Cycle through menus
  if (btnNowMenu == 0 && btnLastMenu == 1)
  {
    nextPage();
    lastPress = millis();
    btnLastMenu = btnNowMenu;
  }
  else
  {
    btnLastMenu = btnNowMenu;
  }

  // Adjust values
  if (btnNowModify == 0 && btnLastModify == 1)
  {
    currentSettingsCount++;
    switch (currentPage)
    {
    case 1:
      nozzleMax = incrementValue(nozzleMax, 150, 300, 10);
      drawPage(1, true);
      break;
    case 2:
      bedMax = incrementValue(bedMax, 50, 150, 10);
      drawPage(2, true);
      break;
    case 3:
      delayMax = incrementValue(delayMax, 1, 10, 1);
      drawPage(3, true);
      break;
    }
    lastPress = millis();
    btnLastModify = btnNowModify;
  }
  else
  {
    btnLastModify = btnNowModify;
  }

  // Reset to dashboard
  if (millis() - lastPress > resetDisplayDelay && currentPage != 0)
  {
    drawPage(0);
  }

  // Redraw Dashboard
  if (currentPage == 0 && (millis()-lastUpdate) > updateDelay){
    drawPage(0, true);
  }

  delay(50);

  // Serial.print("Menu: ");
  // Serial.println(digitalRead(pinMenu));
  // Serial.print("Modify: ");
  // Serial.println(btnNowModify);
  //  delay(1000);

  // nextPage();
  // delay(1000);
}