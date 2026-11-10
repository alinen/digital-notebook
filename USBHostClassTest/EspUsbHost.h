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
  EspUsbHost();
  virtual ~EspUsbHost();
  virtual void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier);
  void setup();
  void tick();
};

#endif

