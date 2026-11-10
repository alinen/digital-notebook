/*
 * Example: USB host — serial (CDC) + flash drive (MSC) + mouse and keyboard (HID)
 *
 * What you need:
 *   - ESP32-S2, S3, or P4 with USB OTG; disable CDC/MSC/DFU on boot when using USB-OTG mode.
 *   - Call USBHost.begin() once, then USBHost.task() often (here: every loop).
 *
 * What this sketch does:
 *   - **CDC:** bytes from a USB serial device appear on the Serial Monitor; typing in the Monitor is sent to the device.
 *   - **MSC:** when a USB flash drive is plugged in, it mounts as a FAT disk (same idea as an SD card).
 *   - **HID:** mouse and keyboard report callbacks print a line on each report.
 *     (Some mice send reports very often; that is normal.)
 *
 * Like TinyUSB’s host demo "cdc_msc_hid", but using the Arduino USB library classes.
 */

#include <Arduino.h>
#include <USBHost.h>
#include <USBHostSerial.h>
#include <USBHostMSC.h>
#include <USBMSCFS.h>
#include <USBHostHID.h>
#include <USBHostHIDMouse.h>
#include <USBHostHIDKeyboard.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "EspUsbHost.h"

class MyEspUsbHost : public EspUsbHost {
  void onKeyUp(char ascii, uint8_t keycode, uint8_t modifier) {
    Serial.printf("key UP: %c (%d)\n", ascii, keycode);
  };

  void onKeyDown(char ascii, uint8_t keycode, uint8_t modifier) {
    Serial.printf("key DOWN: %c (%d)\n", ascii, keycode);
  };
};

MyEspUsbHost usbHost;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.printf(
    "build flags: USB_MODE=%d CDC_ON_BOOT=%d MSC_ON_BOOT=%d DFU_ON_BOOT=%d\n", (int)ARDUINO_USB_MODE, (int)ARDUINO_USB_CDC_ON_BOOT,
    (int)ARDUINO_USB_MSC_ON_BOOT, (int)ARDUINO_USB_DFU_ON_BOOT
  );
  Serial.println(F("(Host: in USB-OTG mode set all *_ON_BOOT=0; use ESP-Prog/UART or Hardware CDC for Serial Monitor.)"));
  Serial.println();

  Serial.println(F("=== USB host: CDC + MSC + HID (starter example) ==="));
  Serial.println(F("Plug a USB serial adapter, a FAT flash drive, and/or a mouse or keyboard (hub is OK)."));
  Serial.println();

  usbHost.setup();
}

void loop() {
  usbHost.tick();
  delay(200);
}
