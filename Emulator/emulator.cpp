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
#define SSD1306_WHITE COLOR_WHITE
#define SSD1306_BLACK COLOR_BLACK
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
#else
#define SD_SCK D8
#define SD_MISO D9
#define SD_MOSI D10
#define SD_CS D2
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SPIClass sdSPI(FSPI);
#endif

// note: cursorpos is for the buffer location
const int PAGE_SIZE = 10;
const int COL_NUM = 21;
const int ROW_NUM = 8 * PAGE_SIZE;

char buffer[ROW_NUM][COL_NUM];

bool editmode = 1;
std::string DIARY_FILE_NAME;

uint8_t bufferRow = 0;
uint8_t bufferCol = 0;

uint8_t cursorRow = 0;
uint8_t cursorCol = 0;

int CURSOR_INTERVAL = 450;
long lastBlinkMs = 0;
bool cursorReady = false;
bool cursorOn = false;

long lastKeyPress = millis();
long keyPressCount = 0;
long lastKeyPressCount = 0;

void printBuffer() {
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text

  int startCol = 0;
  int startRow;
  if (bufferRow <= 7) {
    startRow = 0;
  } else {
    startRow = bufferRow - 7;
  }

  int sr = 0;
  int sc = 0;
  for (int row = startRow; (row < startRow + 8) && (row < ROW_NUM); row++) {
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
  cursorReady = true;
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


void updateFile() {
}

void handleKeypress(uint8_t ascii) {
  if (' ' <= ascii && ascii <= '~') {
    buffer[bufferRow][bufferCol] = ascii;
    bufferCol++;
  } 
  else if (ascii == 13 /* newline */) {
    buffer[bufferRow][bufferCol] = ascii;
    bufferCol = 0;
    bufferRow++;
  } 
  else if (ascii == 8 || ascii == KEY_BACKSPACE || ascii == 7 /* backspace */) {
    if (bufferRow == 0 && bufferCol == 0) {
      return;
    }

    if (bufferCol == 0) {
      bufferRow--;
      bufferCol = COL_NUM - 1;
      while (bufferCol > 0) {
        if (buffer[bufferRow][bufferCol] != '\0') {
          break;
        }
        bufferCol--;
      }
    } else {
      bufferCol--;
    }

    buffer[bufferRow][bufferCol] = ' ';
  }
  else if (ascii == 27 /* escape */) { // show menu
    editmode = 0;
  }
  else if (ascii == KEY_DOWN) { 

  }
  else if (ascii == KEY_UP) { 

  }

  else if (ascii == KEY_LEFT) { 

  }

  else if (ascii == KEY_UP) { 

  }

  lastKeyPress = millis();
  keyPressCount++;

  if (bufferCol >= COL_NUM) {
    bufferCol = 0;
    bufferRow++;
  }
  
  // Handle case where buffer is full
  if (bufferRow >= ROW_NUM) {
    bufferRow = 0;
  }
  cursorRow = bufferRow > 7? 7 : bufferRow;
  cursorCol = bufferCol;

  printBuffer();

  lastKeyPress = millis();
}

std::string openNextFile() {
  return "1.txt";
}

class MyEspUsbHost : public EspUsbHost {
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    handleKeypress(ascii);
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
#endif

  memset(buffer, ' ', COL_NUM * ROW_NUM);
  DIARY_FILE_NAME = openNextFile();

  delay(200);
  std::string message = "Digital Notebook\n\nSaving to ";
  message += DIARY_FILE_NAME + "\n\n:)";
  printToScreen(message.c_str());
}

void loop() {
  while (1) {
    if (keyPressCount == 0) { // clear on first key press
      memset(buffer, ' ', ROW_NUM * COL_NUM);
    }

    usbHost.task();
    if (((millis()-lastKeyPress) > 3000) && (keyPressCount != lastKeyPressCount)){
      updateFile(); // save to secret backup file?
      lastKeyPressCount = keyPressCount;
    }
  }
}

int main() {
  initscr();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();

  setup();
  loop();

  endwin();
  return 0;
}