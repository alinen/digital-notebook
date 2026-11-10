#ifndef __EspUsbHost_H__
#define __EspUsbHost_H__

#include <Arduino.h>
#include <USBHost.h>
#include <USBHostSerial.h>
#include <USBHostMSC.h>
#include <USBMSCFS.h>
#include <USBHostHID.h>
#include <USBHostHIDMouse.h>
#include <USBHostHIDKeyboard.h>

class EspUsbHost {
public:
  EspUsbHost(){}
  virtual ~EspUsbHost() {}
  virtual void onKeyDown(char ascii, uint8_t keycode, uint8_t modifier) {}
  virtual void onKeyUp(char ascii, uint8_t keycode, uint8_t modifier){}
  bool keyIsDown(uint8_t keycode) { return flags[keycode];}
  void setup();
  void tick();

  bool flags[256];
};

#endif

