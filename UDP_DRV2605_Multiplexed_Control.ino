#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include "belt_utils.h"
#include "udp_utils.h"
#include "study_utils.h"

const char *ssid = "ESP32_AP";
const char *password = "12345678";
const int localUdpPort = 4210;

std::array<float,8> received_distances = {1.0f}; // arr for distance floats

float prev_val = 0; // this is for testing in loop()

void setup() {
  Serial.begin(9600);
  setupWireless(ssid, password, localUdpPort);
  Wire.begin();
  Serial.println("Program Start");
  
  if (!setupBelt()) {
    Serial.println("Belt setup failed");
    while(1) {
      delay(100);
    }
  }
  
}

void loop() {
  
  received_distances = getDistanceFloats(received_distances);

  //Below if statement for testing getDistanceFloats
  if (received_distances[0] != prev_val) {
      Serial.print("Received: ");
      Serial.println(received_distances[0]);
      Serial.flush();
      prev_val = received_distances[0];
  }
    

  updateBelt(received_distances, FREQUENCY_CONDITION, MIN_ACTIVATION_DIST, MAX_ACTIVATION_DIST);
  
  
}
