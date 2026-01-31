#include "belt_utils.h"
#include "debug.h"



//Global arrays for pointers to motor driver and pulser objects
namespace {
  Adafruit_DRV2605* drv[NUM_DRIVERS] = {nullptr};
  HapticPulser* pulser[NUM_DRIVERS] = {nullptr};
  unsigned long globalTickMillis;
}

void multiplexSelect(uint8_t i) {
  if (i >= NUM_DRIVERS) return;
 
  Wire.beginTransmission(MULTIPLEX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

bool setupBelt() {
  globalTickMillis = millis();
  for (int i = 0; i < NUM_DRIVERS; i++) {
    if ((i == 1) || (i == 2) || (i == 3)) {
      continue;
    }
    multiplexSelect(i);
    drv[i] = new Adafruit_DRV2605();
    // Start each drv
    if (!drv[i]->begin()) {
      debug("Could not find DRV2605 #"); debugln(i);
      return false;
    }
    delay(100);
    // Create and start each pulser
    pulser[i] = new HapticPulser(*drv[i], i);
    if (! pulser[i]->begin(true, 3.8f, 5.0f)) {
      debug("Could not begin pulser #"); debugln(i);
      return false;
    }
    pulser[i]->start(globalTickMillis);
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
} study_params_struct;

void updateBelt(std::array<float,8>& distances, std::array<float,8>& study_params) {
  
  
  // Only used by duty cycle modulation
  static uint8_t previousCondition = ConditionState::NONE; // condition as of last tick; initialize w/ sentinal value
  static float previousFixedFreqHz = -1;
  //static float previousOnDuration = -1;
  static unsigned long previousCycleStart = 0;
  static unsigned long nextCycleStart = 0;
  static bool beltStateIsOn = false;
  static float fixedPeriodMs;
  static unsigned long pulserOnDurations[NUM_DRIVERS] = {0};

  globalTickMillis = millis();

  //Assign params to easy to understand stuct
  study_params_struct.condition_selection = static_cast<uint8_t>(study_params[0]);
  study_params_struct.min_activation_dist = study_params[1]; 
  study_params_struct.max_activation_dist = study_params[2];
  study_params_struct.min_freq_hz = study_params[3];
  study_params_struct.max_freq_hz = study_params[4];
  study_params_struct.fixed_duty_cycle = study_params[5];
  study_params_struct.fixed_freq_hz = study_params[6];
  study_params_struct.just_detectable_intensity = study_params[7];


  // Logic for making sure cycleStart begins when we change to duty cycle condition
  if (previousCondition == ConditionState::NONE) {
    previousCondition = study_params_struct.condition_selection;
  }
  if ((previousCondition == ConditionState::DUTY_CYCLE_MODULATION) && (study_params_struct.condition_selection == ConditionState::FREQUENCY_MODULATION)) {
    previousCondition = ConditionState::FREQUENCY_MODULATION;
  }
  else if ((previousCondition == ConditionState::FREQUENCY_MODULATION) && (study_params_struct.condition_selection == ConditionState::DUTY_CYCLE_MODULATION)) {
    previousCondition = ConditionState::DUTY_CYCLE_MODULATION;
    unsigned long tmp = globalTickMillis;
    previousCycleStart = tmp;
    nextCycleStart = tmp + hzToPeriodMs(study_params_struct.fixed_freq_hz);
    beltStateIsOn = false;
  }


  if (study_params_struct.condition_selection == ConditionState::FREQUENCY_MODULATION) { //change frequency (mostly handled by each pulser)
    for (int i = 0; i < NUM_DRIVERS; i++) {
      if ((i == 1) || (i == 2) || (i == 3)) {
        continue;
      }
      multiplexSelect(i);
      float activation_percentage = rawDistToActivationPercentage(distances[i], study_params_struct.min_activation_dist, study_params_struct.max_activation_dist);
      modulateIntensity(activation_percentage, pulser[i], study_params_struct.just_detectable_intensity);
      modulatePulseFrequency(activation_percentage, pulser[i], study_params_struct.min_freq_hz, study_params_struct.max_freq_hz, study_params_struct.fixed_duty_cycle);
      pulser[i]->update(globalTickMillis);
    }
  }

  if (study_params_struct.condition_selection == ConditionState::DUTY_CYCLE_MODULATION) { //change duty cycle (mostly handled by belt manager (this))

    // if fixed frequency has changed, update the fixedPeriod we are using
    if (previousFixedFreqHz != study_params_struct.fixed_freq_hz) {
      fixedPeriodMs = hzToPeriodMs(study_params_struct.fixed_freq_hz);
      previousFixedFreqHz = study_params_struct.fixed_freq_hz;
    }

    // If motors are off, and we have passed the flag for nextCycle start, turn all pulsers on
    if ((!beltStateIsOn) && (globalTickMillis >= nextCycleStart)) {
  
      for (int i = 0; i < NUM_DRIVERS; i++) {
        if ((i == 1) || (i == 2) || (i == 3)) {
          continue;
        }
        multiplexSelect(i);
        float activation_percentage = rawDistToActivationPercentage(distances[i], study_params_struct.min_activation_dist, study_params_struct.max_activation_dist);
        pulserOnDurations[i] = activation_percentage * fixedPeriodMs;
        pulser[i]->setState(PulserState::ON);
        modulateIntensity(activation_percentage, pulser[i], study_params_struct.just_detectable_intensity);
      }

      beltStateIsOn = true; 
      previousCycleStart = nextCycleStart;
      nextCycleStart = nextCycleStart + static_cast<unsigned long>(fixedPeriodMs);   
      
    }

    else if (beltStateIsOn) { //some/all pulsers are on, check if we need to turn any of the pulsers off
      
      // While running routine to check if any pulsers need to stop, check if ALL pulsers are off. If so, recalculate nextCycleStart and turn isOnState off
      bool anyPulsersOn = false; // Start assuming they are all off

      for (int i = 0; i < NUM_DRIVERS; i++) { 
        if ((i == 1) || (i == 2) || (i == 3)) {
          continue;
        }
        multiplexSelect(i);

        if (pulser[i]->isOn() && (globalTickMillis > (previousCycleStart + pulserOnDurations[i]))) { //check if we need to turn this pulser off
          pulser[i]->setState(PulserState::OFF);
          pulser[i]->forceOff();
        }
        else if (pulser[i]->isOn()) { // if we don't need to turn it off this time, and it is on, update intensity
          float activation_percentage = rawDistToActivationPercentage(distances[i], study_params_struct.min_activation_dist, study_params_struct.max_activation_dist);
          modulateIntensity(activation_percentage, pulser[i], study_params_struct.just_detectable_intensity);
          anyPulsersOn = true;
        }
        else {
          ; // this means this specific pulser is already off, so we should keep it off until it turns on
        }

      }

      if (!anyPulsersOn) { //if all pulsers are off
        // previousCycleStart = nextCycleStart;
        // nextCycleStart = nextCycleStart + static_cast<unsigned long>(fixedPeriodMs);
        beltStateIsOn = false;
      }

    }
    else {
      ; //this means the entire belt is off, so we can just leave it off until we want to turn it on again
    }

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
  unsigned long periodMs = hzToPeriodMs(freqHz);

  pulser->setOnOff(periodMs * fixed_duty_cycle, periodMs * (1 - fixed_duty_cycle), globalTickMillis);
}

unsigned long hzToPeriodMs(float hz) {
  unsigned long period;
  if (hz > 0.0f) {
          period = 1000.0f / hz;
        }
        else {
          period = 0;
        }
  return period;
}

