#include <Arduino.h>
#include <EEPROM.h>
#include "header.h"
#include <TFT_eSPI.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>

TFT_eSPI myGLCD = TFT_eSPI();

#define DELAY 500

#define TFT_GREY 0x7BEF
#define TFT_W 240
#define TFT_H 135

unsigned long runTime = 0;

#define LED_STATUS 2

#define addr_bedMax 0
#define addr_nozzleMax 2
#define addr_delayMax 4
#define addr_smokeMax 6

#define pinModify 0
#define pinMenu 35

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

int smokeMax;

int delayMax;

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
  writeIntIntoEEPROM(addr_smokeMax, smokeMax);
  blink();
  blink();
}

void blink()
{
  digitalWrite(LED_STATUS, HIGH);
  delay(50);
  digitalWrite(LED_STATUS, LOW);
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
  else if (currentValue == 0)
  {
    return minValue;
  }
  else
  {
    return currentValue + increment;
  }
}

int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void nextPage()
{
  if (currentPage >= 4)
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
      Serial.println((String) "======== Dashboard " + (pageNumber + 1) + "/5 =========");
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
      if (smokeMax == 1)
      {
        Serial.println((String) "Smoke detector    : Enabled");
      }
      else
      {
        Serial.println((String) "Smoke detector    : Disabled");
      }
      break;
    case 1:
      Serial.println((String) "======== Setup Nozzle max Temperature " + (pageNumber + 1) + "/5 ==========");
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
      Serial.println((String) "======== Setup Bed max Temperature " + (pageNumber + 1) + "/5 ==========");
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
      Serial.println((String) "======== Setup Delay Temperature " + (pageNumber + 1) + "/5 ==========");
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
    case 4:
      Serial.println((String) "======== Smoke detector " + (pageNumber + 1) + "/5 ==========");
      if (smokeMax == 1)
      {
        Serial.println((String) "Smoke detector: Enabled");
      }
      else
      {
        Serial.println((String) "Smoke detector: Disabled");
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

  randomSeed(analogRead(A0));

  myGLCD.init();
  myGLCD.setRotation(1);

  pinMode(LED_STATUS, OUTPUT);
  pinMode(pinMenu, INPUT_PULLUP);
  pinMode(pinModify, INPUT_PULLUP);

  btnLastMenu = digitalRead(pinMenu);
  btnLastModify = digitalRead(pinModify);

  nozzleMax = readIntFromEEPROM(addr_nozzleMax);
  bedMax = readIntFromEEPROM(addr_bedMax);
  delayMax = readIntFromEEPROM(addr_delayMax);
  smokeMax = readIntFromEEPROM(addr_smokeMax);

  Serial.println("==== Startup ====");
  drawPage(0, true);
  blink();
  blink();
}

void drawLCD(int pageNumber, bool force)
{

  if (currentPage != pageNumber || force)
  {

    int TEXT_FONT = 2;
    myGLCD.fillScreen(TFT_BLACK);

    myGLCD.fillRect(0, 0, TFT_W - 1, 14, TFT_BLUE);
    myGLCD.fillRect(0, TFT_H - 14, TFT_W - 1, 14, TFT_BLUE);
    myGLCD.setTextColor(TFT_YELLOW, TFT_BLUE);
    myGLCD.drawCentreString("3D Printer - KillSwitch", TFT_W / 2, 4, 1);
    myGLCD.drawCentreString("by Christian Pedersen", TFT_W / 2, TFT_H - 12, 1);

    myGLCD.setTextColor(TFT_WHITE, TFT_BLACK);

    int INDENT = 150;

    myGLCD.drawString("Bed Status", 8, 16+5, TEXT_FONT);
    myGLCD.drawString((String) bedCurrent + " / " + bedMax, INDENT, 16+5, TEXT_FONT);

    myGLCD.drawString("Nozzle Status", 8, 31+5, TEXT_FONT);
    myGLCD.drawString((String) nozzleCurrent + " / " + nozzleMax, INDENT, 31+5, TEXT_FONT);

    myGLCD.drawString("Shutdown Delay", 8, 46+5, TEXT_FONT);
    myGLCD.drawString((String) delayMax + " minutes", INDENT, 46+5, TEXT_FONT);

    myGLCD.drawString("Smokedetector", 8, 61+5, TEXT_FONT);
    myGLCD.drawString("Enabled", INDENT, 61+5, TEXT_FONT);
  }
  delay(500);
}

void lcdStuff()
{
  //randomSeed(millis());
  randomSeed(1234); // This ensure test is repeatable with exact same draws each loop
  int buf[TFT_W - 2];
  int x, x2;
  int y, y2;
  int r;
  runTime = millis();
  // Clear the screen and draw the frame
  myGLCD.fillScreen(TFT_BLACK);

  myGLCD.fillRect(0, 0, TFT_W - 1, 14, TFT_RED);

  myGLCD.fillRect(0, TFT_H - 14, TFT_W - 1, 14, TFT_GREY);

  myGLCD.setTextColor(TFT_BLACK, TFT_RED);
  myGLCD.drawCentreString("* TFT_S6D02A1 *", TFT_W / 2, 4, 1);
  myGLCD.setTextColor(TFT_YELLOW, TFT_GREY);
  myGLCD.drawCentreString("Adapted by Bodmer", TFT_W / 2, TFT_H - 12, 1);

  myGLCD.drawRect(0, 14, TFT_W - 1, TFT_H - 28, TFT_BLUE);

  // Draw crosshairs
  myGLCD.drawLine(TFT_W / 2 - 1, 15, TFT_W / 2 - 1, TFT_H - 16, TFT_BLUE);
  myGLCD.drawLine(1, TFT_H / 2 - 1, TFT_W - 2, TFT_H / 2 - 1, TFT_BLUE);
  for (int i = 9; i < TFT_W - 1; i += 10)
    myGLCD.drawLine(i, TFT_H / 2 - 3, i, TFT_H / 2 + 1, TFT_BLUE);
  for (int i = 19; i < TFT_H - 20; i += 10)
    myGLCD.drawLine(TFT_W / 2 - 3, i, TFT_W / 2 + 1, i, TFT_BLUE);

  // Draw sin-, cos- and tan-lines
  myGLCD.setTextColor(TFT_CYAN);
  myGLCD.drawString("Sin", 5, 15, 2);
  for (int i = 1; i < TFT_W - 2; i++)
  {
    myGLCD.drawPixel(i, TFT_H / 2 - 1 + (sin(((i * 2.26) * 3.14) / 180) * 48), TFT_CYAN);
  }
  myGLCD.setTextColor(TFT_RED);
  myGLCD.drawString("Cos", 5, 30, 2);
  for (int i = 1; i < TFT_W - 2; i++)
  {
    myGLCD.drawPixel(i, TFT_H / 2 - 1 + (cos(((i * 2.26) * 3.14) / 180) * 48), TFT_RED);
  }
  myGLCD.setTextColor(TFT_YELLOW);
  myGLCD.drawString("Tan", 5, 45, 2);
  for (int i = 1; i < TFT_W - 2; i++)
  {
    myGLCD.drawPixel(i, TFT_H / 2 - 1 + (tan(((i * 2.26) * 3.14) / 180)), TFT_YELLOW);
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  myGLCD.drawLine(TFT_W / 2 - 1, 15, TFT_W / 2 - 1, TFT_H - 16, TFT_BLUE);
  myGLCD.drawLine(1, TFT_H / 2 - 1, TFT_W - 2, TFT_H / 2 - 1, TFT_BLUE);
  int col = 0;
  // Draw a moving sinewave
  x = 1;
  for (int i = 1; i < ((TFT_W - 3) * 20); i++)
  {
    x++;
    if (x == TFT_W - 2)
      x = 1;
    if (i > TFT_W - 2)
    {
      if ((x == TFT_W / 2 - 1) || (buf[x - 1] == TFT_H / 2 - 1))
        col = TFT_BLUE;
      else
        myGLCD.drawPixel(x, buf[x - 1], TFT_BLACK);
    }
    y = TFT_H / 2 + (sin(((i * 2.2) * 3.14) / 180) * (49 - (i / 100)));
    myGLCD.drawPixel(x, y, TFT_BLUE);
    buf[x - 1] = y;
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some filled rectangles
  for (int i = 1; i < 6; i++)
  {
    switch (i)
    {
    case 1:
      col = TFT_MAGENTA;
      break;
    case 2:
      col = TFT_RED;
      break;
    case 3:
      col = TFT_GREEN;
      break;
    case 4:
      col = TFT_BLUE;
      break;
    case 5:
      col = TFT_YELLOW;
      break;
    }
    myGLCD.fillRect(30 + (i * 10), 20 + (i * 10), 30, 30, col);
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some filled, rounded rectangles
  for (int i = 1; i < 6; i++)
  {
    switch (i)
    {
    case 1:
      col = TFT_MAGENTA;
      break;
    case 2:
      col = TFT_RED;
      break;
    case 3:
      col = TFT_GREEN;
      break;
    case 4:
      col = TFT_BLUE;
      break;
    case 5:
      col = TFT_YELLOW;
      break;
    }
    myGLCD.fillRoundRect(TFT_W / 2 + 20 - (i * 10), 20 + (i * 10), 30, 30, 3, col);
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some filled circles
  for (int i = 1; i < 6; i++)
  {
    switch (i)
    {
    case 1:
      col = TFT_MAGENTA;
      break;
    case 2:
      col = TFT_RED;
      break;
    case 3:
      col = TFT_GREEN;
      break;
    case 4:
      col = TFT_BLUE;
      break;
    case 5:
      col = TFT_YELLOW;
      break;
    }
    myGLCD.fillCircle(45 + (i * 10), 30 + (i * 10), 15, col);
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some lines in a pattern

  for (int i = 15; i < TFT_H - 16; i += 5)
  {
    myGLCD.drawLine(1, i, (i * 1.44) - 10, TFT_H - 17, TFT_RED);
  }

  for (int i = TFT_H - 17; i > 15; i -= 5)
  {
    myGLCD.drawLine(TFT_W - 3, i, (i * 1.44) - 11, 15, TFT_RED);
  }

  for (int i = TFT_H - 17; i > 15; i -= 5)
  {
    myGLCD.drawLine(1, i, TFT_W + 11 - (i * 1.44), 15, TFT_CYAN);
  }

  for (int i = 15; i < TFT_H - 16; i += 5)
  {
    myGLCD.drawLine(TFT_W - 3, i, TFT_W + 10 - (i * 1.44), TFT_H - 17, TFT_CYAN);
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some random circles
  for (int i = 0; i < 100; i++)
  {
    x = 32 + random(TFT_W - 2 - 32 - 30);
    y = 45 + random(TFT_H - 19 - 45 - 30);
    r = random(30);
    myGLCD.drawCircle(x, y, r, random(0xFFFF));
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some random rectangles
  for (int i = 0; i < 100; i++)
  {
    x = 2 + random(TFT_W - 4);
    y = 16 + random(TFT_H - 33);
    x2 = 2 + random(TFT_H - 4);
    y2 = 16 + random(TFT_H - 33);
    if (x2 < x)
    {
      r = x;
      x = x2;
      x2 = r;
    }
    if (y2 < y)
    {
      r = y;
      y = y2;
      y2 = r;
    }
    myGLCD.drawRect(x, y, x2 - x, y2 - y, random(0xFFFF));
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  // Draw some random rounded rectangles
  for (int i = 0; i < 100; i++)
  {
    x = 2 + random(TFT_W - 4);
    y = 16 + random(TFT_H - 33);
    x2 = 2 + random(TFT_W - 4);
    y2 = 16 + random(TFT_H - 33);
    // We need to get the width and height and do some window checking
    if (x2 < x)
    {
      r = x;
      x = x2;
      x2 = r;
    }
    if (y2 < y)
    {
      r = y;
      y = y2;
      y2 = r;
    }
    // We need a minimum size of 6
    if ((x2 - x) < 6)
      x2 = x + 6;
    if ((y2 - y) < 6)
      y2 = y + 6;
    myGLCD.drawRoundRect(x, y, x2 - x, y2 - y, 3, random(0xFFFF));
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

  //randomSeed(1234);
  int colour = 0;
  for (int i = 0; i < 100; i++)
  {
    x = 2 + random(TFT_W - 4);
    y = 16 + random(TFT_H - 31);
    x2 = 2 + random(TFT_W - 4);
    y2 = 16 + random(TFT_H - 31);
    colour = random(0xFFFF);
    myGLCD.drawLine(x, y, x2, y2, colour);
  }

  delay(DELAY);

  myGLCD.fillRect(1, 15, TFT_W - 3, TFT_H - 31, TFT_BLACK);

#define RANDOM

#ifdef RANDOM
  // Draw 10,000 pixels
  // It takes 30ms to calculate the 30,000 random numbers so this is not a true drawPixel speed test
  for (int i = 0; i < 10000; i++)
  {
    myGLCD.drawPixel(2 + random(TFT_W - 3), 16 + random(TFT_H - 31), random(0xFFFF));
  }
#else
  // Draw 10,000 pixels to fill a 100x100 pixel box, better drawPixel speed test
  // use the coords as the colour to produce the banding
  byte i = 100;
  while (i--)
  {
    byte j = 100;
    while (j--)
      myGLCD.drawPixel(i + TFT_W / 2 - 50, j + TFT_H / 2 - 50, i + j);
  }
#endif

  delay(DELAY);

  myGLCD.fillScreen(TFT_BLUE);
  myGLCD.fillRoundRect(20, 20, TFT_W - 40, TFT_H - 60, 6, TFT_RED);

  myGLCD.setTextColor(TFT_WHITE, TFT_RED);
  myGLCD.drawCentreString("That's it!", TFT_W / 2 - 1, 23, 2);
  myGLCD.drawCentreString("Restarting in a", TFT_W / 2 - 1, 40, 2);
  myGLCD.drawCentreString("few seconds...", TFT_W / 2 - 1, 57, 2);

  runTime = millis() - runTime;
  myGLCD.setTextColor(TFT_GREEN, TFT_BLUE);
  myGLCD.drawCentreString("Draw time: (msecs)", TFT_W / 2, TFT_H - 34, 2);
  myGLCD.drawNumber(runTime - 11 * DELAY, TFT_W / 2 - 20, TFT_H - 17, 2);
  delay(5000);
}

void loop()
{

  drawLCD(0, true);

  bool btnNowMenu = digitalRead(pinMenu);
  bool btnNowModify = digitalRead(pinModify);

  // Save Settings of modified
  if (currentSettingsCount > lastSaveSettingsCount && (millis() - lastPress) > saveDelay)
  {
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
    case 4:
      smokeMax = incrementValue(smokeMax, 1, 1, 1);
      drawPage(4, true);
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
  if (currentPage == 0 && (millis() - lastUpdate) > updateDelay)
  {
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