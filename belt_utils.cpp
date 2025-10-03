#include "belt_utils.h"

namespace {
  Adafruit_DRV2605* drv[NUM_DRIVERS];
  HapticPulser* pulser[NUM_DRIVERS];
}

void multiplexSelect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(MULTIPLEX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

bool setupBelt() {
    for (int i = 0; i < NUM_DRIVERS; i++) {
        multiplexSelect(i);
        drv[i] = new Adafruit_DRV2605();
        // Start each drv
        if (!drv[i]->begin()) {
          Serial.println("Could not find DRV2605 #" + i);
          return false;
        }
        delay(50);
        // Create and start each pulser
        pulser[i] = new HapticPulser(*drv[i]);
        if (! pulser[i]->begin(false, 3.8f, 4.0f)) {
          Serial.println("Could not begin pulser #" + i);
          return false;
        } 
        pulser[i]->start();
    }
    delay(50);
    return true;
}

void updateBelt(std::array<float,8> received_distances, int participant_condition) {
    
    for (int i = 0; i < NUM_DRIVERS; i++) {
    multiplexSelect(i);
    switch (participant_condition) {
      case 1: //change intensity
        modulateIntensity(received_distances[i], pulser[i]);
      case 2: //tbd
        continue;
    }
    
    pulser[i]->update();
    }
}


void modulateIntensity(float distance, HapticPulser *pulser) {
  pulser->setIntensity(distance);
}
