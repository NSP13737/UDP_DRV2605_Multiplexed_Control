#include "belt_utils.h"
#include "debug.h"



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
          debug("Could not find DRV2605 #");
          debugln(i);
          return false;
        }
        delay(100);
        // Create and start each pulser
        pulser[i] = new HapticPulser(*drv[i], i);
        if (! pulser[i]->begin(true, 3.8f, 5.0f)) {
          debug("Could not begin pulser #");
          debugln(i);
          return false;
        }
        pulser[i]->start();
    }
    delay(50);
    return true;
}

//Struct for use in updateBelt to make passing params clearer
struct {
  uint8_t condition_selection;
  float min_activation_dist;
  float max_activation_dist;
  float min_freq_hz;
  float max_freq_hz;
  float fixed_duty_cycle;
  float fixed_freq_hz;
  float just_detectable_intensity;
} study_params_struct ;

void updateBelt(std::array<float,8> distances, std::array<float,8> study_params) {
  static uint8_t lastState = 0; //set to 0 to indicate it hasn't yet been defined by unity
  //Assign params to easy to understand stuct
  study_params_struct.condition_selection = static_cast<uint8_t>(study_params[0]);
  study_params_struct.min_activation_dist = study_params[1]; 
  study_params_struct.max_activation_dist = study_params[2];
  study_params_struct.min_freq_hz = study_params[3];
  study_params_struct.max_freq_hz = study_params[4];
  study_params_struct.fixed_duty_cycle = study_params[5];
  study_params_struct.fixed_freq_hz = study_params[6];
  study_params_struct.just_detectable_intensity = study_params[7];

  
  // If last state hasn't been defined (at start of program), set it to the current condition
  if (lastState == 0) {
    debugln(lastState);
    debugln(study_params_struct.condition_selection);
    lastState = study_params_struct.condition_selection;
  }
  // If the condition state changes either way, sync the belt
  if ((lastState == 2) && (study_params_struct.condition_selection == 1)) {
    lastState = 1;
    syncBelt();
  }
  else if ((lastState == 1) && (study_params_struct.condition_selection == 2)) {
    lastState = 2;
    syncBelt();
  }

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

void syncBelt(void) {
  unsigned long fixed_time = millis();
  for (int i = 0; i < NUM_DRIVERS; i++) {
    debug("Next on time for ");
    debug(i);
    debug(": ");
    multiplexSelect(i);
    pulser[i]->setNextOnTime(fixed_time+10000);
  }
}

