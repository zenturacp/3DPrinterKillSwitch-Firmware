// Draw page - takes option with page numer
void drawPage(int pageNumber, bool force = false);
// Cycle through pages
void nextPage();
// Blink
void blink();
// EEPROM
int readIntFromEEPROM(int address);
void writeIntIntoEEPROM(int address, int number);
void saveSettings();