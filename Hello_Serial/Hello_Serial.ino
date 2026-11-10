/*
 */

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("Hello World"));
}

void loop() {
    delay(2);
    Serial.println("Hi!!");
}
