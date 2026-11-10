#include "EspUsbHost.h"

//USBHostSerialClass USBHostSerial;
//USBMSCFSClass USBMSCFS;
USBHostHIDKeyboard USBHostKeyboard;

static void onKeyboardReport(uint8_t modifiers, const uint8_t keys[6], void *usrdata) {
  EspUsbHost* host = (EspUsbHost*) usrdata;

  // Return early if no input?
  bool any = modifiers != 0;
  for (int i = 0; i < 6 && !any; i++) {
    any = (keys[i] != 0);
  }
  if (!any) {
    USBHostKeyboard.clear();
    return;
  }

  char ascii[7] = "ciao";
  size_t numkeys = USBHostKeyboard.toAscii(ascii, 7, modifiers, keys, nullptr); 
  Serial.printf("result: %d\n", numkeys);
  Serial.printf("ascii: %s\n", ascii);
  Serial.printf("modifier: %d\n", modifiers);

  for (int i = 0; i < numkeys; i++) {
    host->onKeyboardKey(ascii[i], keys[i], modifiers);
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

  //USBHostSerial.begin(kHostSerialBaud);
  Serial.println(F("Host started. Open the Serial Monitor and interact with USB devices."));
}

void EspUsbHost::tick() {
  USBHost.task();
}
