// https://github.com/espressif/esp-idf/blob/v6.0/examples/cxx/pthread/main/cpp_pthread.cpp
// https://docs.arduino.cc/libraries/sd/#File%20class

#ifdef EMULATE
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "EspUsbHostStub.h"
#include "AdafruitStub.h"
#include "SDStub.h"
#else
#include "EspUsbHost.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif
#include <string>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#ifdef EMULATE
#define SSD1306_WHITE 1
#define SSD1306_BLACK 2
#define NBDIR "./digital_notebook/"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
SDStub SD;
#else
#define NBDIR "/digital_notebook/"
#define SD_SCK D8
#define SD_MISO D9
#define SD_MOSI D10
#define SD_CS D2
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SPIClass sdSPI(FSPI);
#endif

const int COL_NUM = 21;
const int ROW_NUM = 8;
char buffer[ROW_NUM][COL_NUM];
char options[ROW_NUM][COL_NUM];
char menu[ROW_NUM][COL_NUM+1] = { // +1 is for trailing `\0'
  "                     ",
  "     __________      ",
  "    |  NEW     |     ",
  "    |  SAVE    |     ",
  "    |  OPEN    |     ",
  "    |__________|     ",
  "                     ",
  "                     "
};

bool showmenu = 0;
int menuSelected = 0;
std::string DIARY_FILE_NAME;

uint8_t bufferRow = 0;
uint8_t bufferCol = 0;

uint8_t cursorRow = 0;
uint8_t cursorCol = 0;

long lastKeyPress = millis();
long keyPressCount = 0;
long lastKeyPressCount = 0;

void printBuffer() {
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0,0);

  int sr = 0;
  int sc = 0;
  for (int row = 0; row < ROW_NUM; row++) {
    for (int col = 0; col < COL_NUM; col++) {
      display.drawChar(sr, sc++, buffer[row][col]);
      if (sc >= COL_NUM) {
        sr++;
        sc = 0;
      }
    }
  }

  display.setCursor(cursorRow, cursorCol);
  display.display();
}

void printMenu() {
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setCursor(0,0);

  int sr = 0;
  int sc = 0;
  for (int row = 0; row < ROW_NUM; row++) {
    if (menuSelected+3 == row) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Draw black text
    }
    else {
      display.setTextColor(SSD1306_WHITE);  // Draw white text
    }
    for (int col = 0; col < COL_NUM; col++) {
      display.drawChar(sr, sc++, menu[row][col]);
      if (sc >= COL_NUM) {
        sr++;
        sc = 0;
      }
    }
  }

  display.display();
}

void printToScreen(const char *s) {
  int r = 0;
  int c = 0;
  for (int i = 0; i < (int) strlen(s); i++){
    buffer[r][c++] = s[i];
    if (c >= COL_NUM) {
      r++;
      c = 0;
    }
  }
  printBuffer();
  display.setCursor(0,0);
}

std::string initialize() {
  if (!SD._exists(NBDIR)) {
    // initialize digital notebook files
    SD._mkdir(NBDIR);
    File settings = SD._open(NBDIR ".nbsettings", FILE_WRITE);
    settings._write((const uint8_t*) "1", 1);
    settings._close();
    return "1.txt";
  }
  char line[32];
  File settings = SD._open(NBDIR ".nbsettings", FILE_WRITE);
  settings._seek(0);
  settings._read((uint8_t*) line, 32);
  int num = atoi(line) + 1;
  snprintf(line, 32, "%d", num);
  settings._seek(0);
  settings._write((const uint8_t*) line, strlen(line));

  char filename[256];
  snprintf(filename, 256, NBDIR "%d.txt", num);
  return filename;
}

void updateFile() {
}

void saveFile() {
}

void openFile() {
}

void backspaceChar() {
  buffer[bufferRow][--bufferCol] = ' ';
  if (bufferCol < 0) {
    bufferRow = bufferRow > 0? bufferRow-1 : 0;
    bufferCol = COL_NUM-1;
  } 
  cursorRow = bufferRow;
  cursorCol = bufferCol;
  printBuffer();
}

void newlineChar() {
  buffer[bufferRow][bufferCol] = '\n';
  bufferCol = cursorCol = 0;
  bufferRow++;
  cursorRow = bufferRow;
  printBuffer();
}

void writeChar(uint8_t ascii) {
  buffer[bufferRow][bufferCol] = ascii;
  bufferCol++;
  if (bufferCol >= COL_NUM) {
    bufferCol = 0;
    bufferRow++;
  }
  cursorCol = bufferCol;
  cursorRow = bufferRow;
  printBuffer();
}

void showMenu() {
  if (showmenu) {
    printBuffer(); 
    showmenu = false;
  }
  else {
    printMenu();
    showmenu = true;
  }
}

void handleKeypress(uint8_t ascii, uint8_t keycode) {
  if (' ' <= ascii && ascii <= '~') writeChar(ascii);
  else if (ascii == 13 /* newline */) newlineChar();
  else if (ascii == 8 /* backspace */) backspaceChar();
  else if (ascii == 27 /* escape */)  showMenu();
  else if (keycode == KEY_DOWN) { 
    menuSelected = (menuSelected + 1) % 3;
  }
  else if (keycode == KEY_UP) { 
    menuSelected = (menuSelected - 1) % 3;
  }
  else if (keycode == KEY_LEFT) { 

  }
  else if (keycode == KEY_UP) { 

  }

  lastKeyPress = millis();
  keyPressCount++;
  // TODO: Handle case where buffer is full

  lastKeyPress = millis();
}

class MyEspUsbHost : public EspUsbHost {
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    handleKeypress(ascii, keycode);
  };
};

MyEspUsbHost usbHost;

void setup() {

#ifndef EMULATE
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

  usbHost.begin();
  usbHost.setHIDLocal(HID_LOCAL_US);
#else
  display.begin(0,0);
#endif

  memset(buffer, ' ', COL_NUM * ROW_NUM);
  DIARY_FILE_NAME = initialize();

  delay(200);
  std::string message = "Digital Notebook\n\nSaving to ";
  message += DIARY_FILE_NAME + "\n\n:)";
  printToScreen(message.c_str());
}

void loop() {
  if (keyPressCount == 0) { // clear on first key press
    memset(buffer, ' ', ROW_NUM * COL_NUM);
  }

  usbHost.task();
  if (((millis()-lastKeyPress) > 3000) && (keyPressCount != lastKeyPressCount)){
    updateFile(); // save to secret backup file? Put in different thread
    lastKeyPressCount = keyPressCount;
  }
}

int main() {
  initscr();
  keypad(stdscr, TRUE);
  noecho();
  start_color();
  cbreak();

  setup();
  while (1) {
    loop();
  }

  endwin();
  return 0;
}