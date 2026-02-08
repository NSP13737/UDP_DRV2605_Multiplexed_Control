// Arduino IDE sketch wrapper generated from src/main.cpp
// Keep the existing `src/` and `include/` folders; open this file in Arduino IDE.

#include <Arduino.h>
#include <Wire.h>
#include <array>

// Use explicit relative includes to pick headers from the include/ folder
#include "src/Adafruit_DRV2605.h"
#include "src/belt_utils.h"
#include "src/udp_utils.h"
#include "src/debug.h"

const char *ssid = "ESP32_AP";
const char *password = "12345678";
const int localUdpPort = 4210;

std::array<float,16> received_data = {};
std::array<float,8> received_distances = {}; // arr for distance floats
std::array<float,8> received_study_params = {};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  
  udpSetupWireless(ssid, password, localUdpPort);
  delay(1000);    
  Wire.begin();
  delay(1000);    
  debugln("Program Start");
  delay(1000);    
  digitalWrite(LED_BUILTIN, HIGH);
  
  if (!setupBelt()) {
    debugln("Belt setup failed");
    while(1) {
      delay(100);
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
  
}

int ledCount = 0;

void loop() {
  
  received_data = getData(received_data);
  std::copy(received_data.begin(), received_data.begin()+received_distances.size(), received_distances.begin()); //copy portion from start to length of distance array
  std::copy(received_data.begin()+received_distances.size(), received_data.begin()+received_distances.size()+received_study_params.size(), received_study_params.begin()); //copy from length of dist to length of dist arr + length of study_params arr
  
  updateBelt(received_distances, received_study_params);

  if (ledCount < 1000) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    ledCount++;
  }
  else if ((ledCount > 1000) && (ledCount < 2000)) {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    ledCount++;
  }
  else if (ledCount > 2000) {
    ledCount = 0;
  }

  // digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  // delay(1000);                      // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  // delay(1000);     
  // debugln("yeah");


}
