#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include "belt_utils.h"
#include "udp_utils.h"
#include "debug.h"

const char *ssid = "ESP32_AP";
const char *password = "12345678";
const int localUdpPort = 4210;

std::array<float,16> received_data = {};
std::array<float,8> received_distances = {}; // arr for distance floats
std::array<float,8> received_study_params = {};

float prev_val = 0; // this is for testing in loop()

void setup() {
  Serial.begin(9600);
  setupWireless(ssid, password, localUdpPort);
  Wire.begin();
  debugln("Program Start");
  
  if (!setupBelt()) {
    debugln("Belt setup failed");
    while(1) {
      delay(100);
    }
  }
  
}

void loop() {
  
  received_data = getData(received_data);
  std::copy(received_data.begin(), received_data.begin()+received_distances.size(), received_distances.begin()); //copy portion from start to length of distance array
  std::copy(received_data.begin()+received_distances.size(), received_data.begin()+received_distances.size()+received_study_params.size(), received_study_params.begin()); //copy from length of dist to length of dist arr + length of study_params arr


  //Below block for testing 
  if (received_data[0] != prev_val) {
      debug("Received: ");
      debugln(received_data[0]);
      debugflush();
      prev_val = received_data[0];
  }
    

  updateBelt(received_distances, received_study_params);
  
  
}
