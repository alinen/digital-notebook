#include "EspUsbHost.h"

USBHostSerialClass USBHostSerial;
USBMSCFSClass USBMSCFS;
USBHostHIDKeyboard USBHostKeyboard;

static void onKeyboardReport(uint8_t modifiers, const uint8_t keys[6], void *usrdata) {
  EspUsbHost* host = (EspUsbHost*) usrdata;

  bool any = modifiers != 0;
  for (int i = 0; i < 6 && !any; i++) {
    any = (keys[i] != 0);
    if (any) host->onKeyboardKey((char) keys[i], keys[i], modifiers);
  }
  if (!any) {
    USBHostKeyboard.clear();
    return;
  }
  
  Serial.print(F("[keyboard] "));
  USBHostKeyboard.printReport(Serial, modifiers, keys);
  Serial.println();
  USBHostKeyboard.clear();
}


EspUsbHost::EspUsbHost() {}
EspUsbHost::~EspUsbHost() {}

void EspUsbHost::onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
}

void EspUsbHost::setup() {
  USBHostKeyboard.registerWithHost();
  USBHostKeyboard.setNotifyOnChangeOnly(true);
  USBHostKeyboard.setReportCallback(onKeyboardReport, this);

  if (!USBHost.begin()) {
    Serial.println(F("USBHost.begin() failed."));
    return;
  }

  Serial.println(F("Host started. Open the Serial Monitor and interact with USB devices."));
}

void EspUsbHost::tick() {
  USBHost.task();
}
