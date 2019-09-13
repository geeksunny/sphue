#ifdef SPHUE_EXAMPLE_PROJECT
#include <Arduino.h>
#include "WifiTools.h"
#include "Sphue.h"

void setup() {
  Serial.begin(BAUD_RATE);
  delay(3000);  // Give serial console a chance to get ready
  Serial.println("Serial console ready.");

  bool connected = wifi_tools::startClient();
  delay(5);
  if (connected) {
    sphue::autoDiscoverHub();
  } else {
    Serial.println("Couldn't connect to wifi!");
  }
}

void loop() {
  // TODO: Perform example operations
}

#endif