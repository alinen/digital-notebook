#ifndef AdafruitStub_H_
#define AdafruitStub_H_

#include <ncurses.h>
#include <sys/time.h>
#include <unistd.h>

unsigned int millis() {
  struct timeval tval;
  gettimeofday(&tval, NULL);
  return tval.tv_sec*1000 + tval.tv_usec;
}

void delay(int ms) {
  sleep(ms/1000.0);
}

class Serial {
public:
  static void begin(int v) {}
  static void printf(const char* s, va_list args) { }
};

class Adafruit_SSD1306 {
public:
  Adafruit_SSD1306(int w, int h) {
    width = w;
    height = h;
  }

  ~Adafruit_SSD1306() {

  }

  bool begin(int a, int b) {
    return true;
  }

  void clearDisplay() {
    erase();
  }

  void setTextSize(int s) {}
  void setTextColor(int fg) {}
  void setTextColor(int fg, int bg) {}
  void setCursor(int y, int x) {
    move(y, x);
  }

  void drawChar(int r, int c, char ch) {
    mvaddch(r, c, ch);
  }

  void print(int r, int c, const char* s) {
    mvprintw(r, c, "%s", s);
  }

  void display() {
    refresh();
  }

private:
  int width;
  int height;
};

#endif