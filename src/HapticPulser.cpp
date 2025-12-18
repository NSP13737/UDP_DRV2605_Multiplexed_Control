#include "HapticPulser.h"
#include <cmath> // For std::round()
#include "debug.h"


// Assumes signed RTP input (128-255)
uint8_t pctToRtp(float percent) {
  if (percent <= 0.0f) return 128;
  if (percent >= 1.0f) return 255;
  return (uint8_t)(round(percent * 128.0f)+128);
}

// Assumes unsigned RTP input (0-255) NOTE THIS WOULD NEED TO BE CHANGED TO USE 0-1 RANGE INSTEAD OF 0-100 RANGE
// uint8_t pctToRtp(float percent) {
//   if (percent <= 0.0f) return 0;
//   if (percent >= 100.0f) return 255;
//   return (uint8_t)(round((percent / 100.0f)*255.0f));
// }

HapticPulser::HapticPulser(Adafruit_DRV2605 &d, uint8_t id) : drv(d), motorId(id), state(IDLE) {}

bool HapticPulser::begin(bool doAutoCal, float ratedVoltage, float odClamp) {
  intensityPct = 0.0f;
  onMs = 500;
  offMs = 500;
  drv.setMode(DRV2605_MODE_REALTIME);

  if (doAutoCal) {
    // call the member directly; can also write this->runAutoCal(...)
    bool ok = this->runAutoCal(ratedVoltage, odClamp);
    if (!ok) {
      // handle failure: return false so caller can react
      return false;
    }
  }

  return true;
}

void HapticPulser::start() {
  if (state == IDLE) {
    drv.setRealtimeValue(pctToRtp(intensityPct));
    state = ON;
    nextToggle = millis() + onMs;
  }
}

void HapticPulser::stop() {
  drv.setRealtimeValue(128); // neutral
  drv.setMode(DRV2605_MODE_INTTRIG);
  state = IDLE;
}

void HapticPulser::update() {

  

  if (state == IDLE) return;

  if (millis() < nextToggle) return;

  else if (state == ON) {
    drv.setRealtimeValue(128); // neutral
    state = OFF;
    lastStateChange = millis();
    nextToggle = lastStateChange + offMs;
    
  } else if (state == OFF) {
    drv.setRealtimeValue(pctToRtp(intensityPct));
    state = ON;
    lastStateChange = millis();
    
    // TEST CODE
    // ----------------------------
    // debug("Motor ");
    // debug(motorId);
    // debug(" ON @: ");
    // debugln(lastStateChange);
    // if (motorId == 7) {
    //   debugln();
    // }
    // -----------------------------

    nextToggle = lastStateChange + onMs;
    
  }
}

bool HapticPulser::isOn() {
  if (state == ON) {
    return true;
  }
  return false;
}

void HapticPulser::setIntensity(float pct) {
  intensityPct = pct;
  if (state == ON) {
    drv.setRealtimeValue(pctToRtp(intensityPct));
  }
}

void HapticPulser::setOnOff(unsigned long onMs_, unsigned long offMs_) {
  onMs = onMs_;
  offMs = offMs_;

  if (state != IDLE) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastStateChange;

    if (state == ON) {
      //If elapsedTime is longer than new on time, change state immediately
      if (elapsedTime >= onMs) {
        nextToggle = currentTime;
      } else {
        nextToggle = lastStateChange + onMs;
      }
      
    } else if (state == OFF) {
      //If elapsedTime is longer than new off time, change state immediately
      if (elapsedTime >= offMs) {
        nextToggle = currentTime;
      } else {
        nextToggle = lastStateChange + offMs;
      }
    }

  }

}

void HapticPulser::setNextOnTime(unsigned long time) {
    drv.setRealtimeValue(128); // neutral
    state = OFF;
    lastStateChange = millis();
    nextToggle = time;
    debugln(nextToggle);
}

