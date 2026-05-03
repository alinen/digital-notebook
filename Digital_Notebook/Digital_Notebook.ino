#include "EspUsbHost.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>
#include "fileview.h"
#include "Menu.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NBDIR "/digital_notebook/"
#define SD_SCK D8
#define SD_MISO D9
#define SD_MOSI D10
#define SD_CS D2

SPIClass sdSPI(FSPI);

// The default graphics font is a 6px by 8px font.
// Our 128px screen can fit 21 chars across (21 * 6px = 126px out of 128),
// and can display 8 lines (8 * 8px = 64px out of 64).
const int COL_NUM = 21;
const int ROW_NUM = 8;
String DIARY_FILE_NAME;
dnb::Fileview fv;
dnb::Menu menu;
bool showmenu = false;

long lastKeyPress = millis();
long keyPressCount = 0;
long lastKeyPressCount = 0;

void printToScreen(const char *s) {
  Serial.printf("Displaying: %s\n", s);
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.print(s);
  display.display();
}


void newFile() {
  char line[32];
  File settings = SD.open(NBDIR ".nbsettings", FILE_WRITE);
  settings.seek(0);
  settings.read((uint8_t*) line, 32);
  int num = atoi(line) + 1;
  snprintf(line, 32, "%d", num);
  settings.seek(0);
  settings.write((const uint8_t*) line, strlen(line));

  char filename[256];
  snprintf(filename, 256, NBDIR "%d.txt", num);
  DIARY_FILE_NAME = filename;

  fv.empty();
}

void saveFile() {
  File file = SD.open(DIARY_FILE_NAME, FILE_WRITE);
  file.seek(0);
  fv.save(file);
  file.close();
}

void initializeNotebookDir() {
  if (!SD.exists(NBDIR)) {
    // initialize digital notebook files
    SD.mkdir(NBDIR);
    File settings = SD.open(NBDIR ".nbsettings", FILE_WRITE);
    settings.write((const uint8_t*) "0", 1);
    settings.close();
  }
}

class MyEspUsbHost : public EspUsbHost {
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    if (ascii == 27) { // escape
      showmenu = !showmenu;
    }
    else if (ascii == 13 && showmenu) {
      int option = menu.getOption();
      if (option == 0) newFile();
      else if (option == 1) saveFile();
      showmenu = false;
    }

    if (showmenu) {
      menu.processChar(ascii, keycode);
      menu.render(display, COL_NUM, ROW_NUM);
    }
    else {
      fv.processChar(ascii, keycode);
      fv.render(display, COL_NUM, ROW_NUM);
    }
    lastKeyPress = millis();
    keyPressCount++;
  };
};

MyEspUsbHost usbHost;

void setup() {

  Serial.begin(115200);
  delay(1000);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }

  printToScreen("setting up sd card");

  delay(200);

  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, sdSPI)) {
    printToScreen("SD card \ninitialization failed!");
    return;
  }

  initializeNotebookDir();
  newFile();

  usbHost.begin();
  usbHost.setHIDLocal(HID_LOCAL_US);
  delay(200);
  printToScreen(("Digital Notebook\n\nSaving to " + 
    DIARY_FILE_NAME.substring(18) + "\n\n" + ":)").c_str());
}

void loop() {
  usbHost.task();
  if (((millis()-lastKeyPress) > 3000) && (keyPressCount != lastKeyPressCount)){
    saveFile();
    lastKeyPressCount = keyPressCount;
  }
}