#include "belt_utils.h"



//Global arrays for pointers to motor driver and pulser objects
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
        delay(6);
        // Create and start each pulser
        pulser[i] = new HapticPulser(*drv[i], i);
        if (! pulser[i]->begin(true, 3.8f, 5.0f)) {
          Serial.println("Could not begin pulser #" + i);
          return false;
        }
        pulser[i]->start();
    }
    delay(50);
    return true;
}

//Struct for use in updateBelt to make passing params clearer
struct StudyParamsStruct {
  int condition_selection;
  float min_activation_dist;
  float max_activation_dist;
  float min_freq_hz;
  float max_freq_hz;
  float fixed_duty_cycle;
  float fixed_freq_hz;
  float just_detectable_intensity;
};

void updateBelt(std::array<float,8> distances, std::array<float,8> study_params) {
  StudyParamsStruct study_params_struct(static_cast<int>(study_params[0]), study_params[1], study_params[2], study_params[3], study_params[4], study_params[5], study_params[6], study_params[7]);
  
  for (int i = 0; i < NUM_DRIVERS; i++) {
    multiplexSelect(i);
    float activation_percentage = rawDistToActivationPercentage(distances[i], study_params_struct.min_activation_dist, study_params_struct.max_activation_dist);
    modulateIntensity(activation_percentage, pulser[i], study_params_struct.just_detectable_intensity);
    
    // Switch based on condition chosen
    switch (study_params_struct.condition_selection) {
      case 1: //change frequency
        modulatePulseFrequency(activation_percentage, pulser[i], study_params_struct.min_freq_hz, study_params_struct.max_freq_hz, study_params_struct.fixed_duty_cycle);
        break;
      case 2: //change duty cycle 
        modulatePulseDutyCycle(activation_percentage, pulser[i], study_params_struct.fixed_freq_hz);
        break;
    }
    pulser[i]->update();
  }
}

float rawDistToActivationPercentage(float distance, float min_activation_dist, float max_activation_dist) {
  if (distance >= max_activation_dist) {
    return 0;
  }
  if (distance <= min_activation_dist) {
    return 1;
  }
  return ((distance-max_activation_dist)/(min_activation_dist-max_activation_dist))    *    ((distance-max_activation_dist)/(min_activation_dist-max_activation_dist)); 

}

void modulateIntensity(float activation_percentage, HapticPulser *pulser, float just_detectable_intensity) {
  //Set to 0 if distance is not in range (without this, when dist is out of range, motor will activate at just_detectable_intensity)
  if (activation_percentage == 0.0f) {
    pulser->setIntensity(0.0f);
  }
  //Otherwise use normally
  else {
    pulser->setIntensity(((activation_percentage-1)*(1-just_detectable_intensity))+1);
  }
  
}

void modulatePulseFrequency(float activation_percentage, HapticPulser *pulser, float min_freq_hz, float max_freq_hz, float fixed_duty_cycle) {
  float freqHz = ((activation_percentage)*(max_freq_hz - min_freq_hz)) + min_freq_hz;
  float periodMs = 1000.0 / freqHz;

  pulser->setOnOff(periodMs * fixed_duty_cycle, periodMs * (1 - fixed_duty_cycle));
}

void modulatePulseDutyCycle(float activation_percentage, HapticPulser *pulser, float fixed_freq_hz) {
  float fixed_period_ms = 1000.0f / fixed_freq_hz;
  pulser->setOnOff(fixed_period_ms * activation_percentage, (fixed_period_ms - (fixed_period_ms * activation_percentage)));
}

