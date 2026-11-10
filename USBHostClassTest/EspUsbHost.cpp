#include "EspUsbHost.h"

//USBHostSerialClass USBHostSerial;
//USBMSCFSClass USBMSCFS;
USBHostHIDKeyboard USBHostKeyboard;

static int inKeys(int keyCode, const uint8_t keys[6]) {
  for (int i = 0; i < 6; i++) {
    if (keys[i] == keyCode) return i;
  }
  return -1;
}

static void onKeyboardReport(uint8_t modifiers, const uint8_t keys[6], void *usrdata) {
  EspUsbHost* host = (EspUsbHost*) usrdata;

  Serial.println("-----------------");
  for (int i = 0; i < 6; i++) {
    Serial.printf("raw key %d: %d\n", i, keys[i]);
  }
  
  char ascii[7];
  size_t numkeys = USBHostKeyboard.toAscii(ascii, 7, modifiers, keys, nullptr); 

  for (int i = 4; i < 256; i++) {
    int idx = inKeys(i, keys);
    if (idx != -1 && host->flags[i] == false) {
      host->onKeyDown(ascii[idx], keys[idx], modifiers);
      host->flags[i] = true;
    }
    else if (idx == -1 && host->flags[i] == true) {
      host->onKeyUp(' ', i, modifiers);
      host->flags[i] = false;
    }
  } 

  Serial.printf("result: %d\n", numkeys);
  Serial.printf("ascii: %s\n", ascii);
  Serial.printf("modifier: %d\n", modifiers);

  
  Serial.print(F("[keyboard] "));
  USBHostKeyboard.printReport(Serial, modifiers, keys);
  Serial.println();
  USBHostKeyboard.clear();
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
