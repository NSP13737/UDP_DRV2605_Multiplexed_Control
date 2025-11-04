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

void updateBelt(std::array<float,8> receivedDistances, int participantCondition, float minActivationDist, float maxActivationDist) {
    
  for (int i = 0; i < NUM_DRIVERS; i++) {
    multiplexSelect(i);
    float activationPercentage = rawDistToActivationPercentage(receivedDistances[i], minActivationDist, maxActivationDist);
    modulateIntensity(activationPercentage, pulser[i]);
    switch (participantCondition) {
      case 1: //change frequency
        modulatePulseFrequency(activationPercentage, pulser[i]);
        break;
      case 2: //change duty cycle 
        modulatePulseDutyCycle(activationPercentage, pulser[i]);
        break;
    }

    pulser[i]->update();
  }
}

float rawDistToActivationPercentage(float distance, float minActivationDist, float maxActivationDist) {
  if (distance >= maxActivationDist) {
    return 0;
  }
  if (distance <= minActivationDist) {
    return 1;
  }
  return ((distance-maxActivationDist)/(minActivationDist-maxActivationDist))    *    ((distance-maxActivationDist)/(minActivationDist-maxActivationDist)); 

}

void modulateIntensity(float activationPercentage, HapticPulser *pulser) {
  pulser->setIntensity(activationPercentage);
}

void modulatePulseDutyCycle(float activationPercentage, HapticPulser *pulser) {

  pulser->setOnOff(FIXED_PERIOD_MS * activationPercentage, (FIXED_PERIOD_MS - (FIXED_PERIOD_MS * activationPercentage)));
}

void modulatePulseFrequency(float activationPercentage, HapticPulser *pulser) {
  float freqHz = ((activationPercentage)*(MAX_FREQ_HZ - MIN_FREQ_HZ)) + MIN_FREQ_HZ;
  float periodMs = 1000.0 / freqHz;

  pulser->setOnOff(periodMs * FIXED_DUTY_CYCLE, periodMs * (1 - FIXED_DUTY_CYCLE));
}