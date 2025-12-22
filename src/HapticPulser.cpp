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
  onMs = 2000;
  offMs = 2000;
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

void HapticPulser::start(unsigned long tickMillis) {
  if (state == IDLE) {
    this->setNextOnTime(20000, tickMillis);
    // drv.setRealtimeValue(pctToRtp(intensityPct));
    // state = ON;
    // nextToggle = millis() + onMs;
  }
}

void HapticPulser::stop() {
  drv.setRealtimeValue(128); // neutral
  drv.setMode(DRV2605_MODE_INTTRIG);
  state = IDLE;
}

void HapticPulser::update(unsigned long tickMillis) {


  if (state == IDLE) return;

  if (tickMillis < nextToggle) return;

  else if (state == ON) {
    drv.setRealtimeValue(128); // neutral
    state = OFF;
    lastStateChange = tickMillis;
    nextToggle = lastStateChange + offMs;
    
  } else if (state == OFF) {
    drv.setRealtimeValue(pctToRtp(intensityPct));
    state = ON;
    lastStateChange = tickMillis;
    
    // TEST CODE
    // ----------------------------
    debug("Motor ");
    debug(motorId);
    debug(" nextToggle is: ");
    debugln(nextToggle);
    if (motorId == 7) {
      debugln();
    }
    // -----------------------------

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

void HapticPulser::setOnOff(unsigned long onMs_, unsigned long offMs_, unsigned long tickMillis) {
  onMs = onMs_;
  offMs = offMs_;

  if (state == IDLE) return;
  unsigned long elapsedTime = tickMillis - lastStateChange;

  if (state == ON) {
    // If elapsedTime is longer than new on time, schedule immediate toggle
    if (elapsedTime >= onMs) {
      nextToggle = tickMillis;
    } else {
      nextToggle = lastStateChange + onMs;
    }
  } else { // state == OFF
    // If elapsedTime is longer than new off time, schedule immediate toggle
    if (elapsedTime >= offMs) {
      nextToggle = tickMillis;
    } else {
      nextToggle = lastStateChange + offMs;
    }
  }

}


void HapticPulser::setOnOffPreservePhase(unsigned long onMs_, unsigned long offMs_, unsigned long tickMillis) {
  // If idle just set durations
  if (state == IDLE) {
    onMs = onMs_;
    offMs = offMs_;
    return;
  }

  unsigned long oldOn = onMs;
  unsigned long oldOff = offMs;
  unsigned long oldPeriod = oldOn + oldOff;

  // If previous period is zero or invalid, fall back to existing behavior
  if (oldPeriod == 0) {
    setOnOff(onMs_, offMs_, tickMillis);
    return;
  }

  // Compute start of the last ON safely (handle potential underflow)
  unsigned long lastOnStart;
  if (state == ON) {
    lastOnStart = lastStateChange;
  } else { // OFF
    if (lastStateChange >= oldOn) {
      lastOnStart = lastStateChange - oldOn;
    } else {
      // handle wrap-around explicitly
      lastOnStart = (ULONG_MAX - (oldOn - lastStateChange) + 1UL);
    }
  }

  // How far into the old cycle we are (unsigned arithmetic handles wrapping)
  unsigned long timeSinceCycleStart = tickMillis - lastOnStart;
  unsigned long phasePos = (oldPeriod == 0) ? 0 : (timeSinceCycleStart % oldPeriod);
  float phase = (float)phasePos / (float)oldPeriod;

  unsigned long newOn = onMs_;
  unsigned long newOff = offMs_;
  unsigned long newPeriod = newOn + newOff;
  if (newPeriod == 0) {
    // defensively set and return
    onMs = newOn;
    offMs = newOff;
    return;
  }

  // Map fractional phase into new period (rounded) and clamp
  unsigned long newPos = (unsigned long)(phase * newPeriod + 0.5f);
  if (newPos >= newPeriod) newPos = newPeriod - 1;

  unsigned long newLastOnStart = tickMillis - newPos;

  // Apply new phase-consistent state and scheduling
  onMs = newOn;
  offMs = newOff;
  if (newPos < newOn) {
    state = ON;
    lastStateChange = newLastOnStart;
    drv.setRealtimeValue(pctToRtp(intensityPct));
    nextToggle = lastStateChange + onMs;
  } else {
    state = OFF;
    lastStateChange = newLastOnStart + newOn;
    drv.setRealtimeValue(128); // neutral while OFF
    nextToggle = lastStateChange + offMs;
  }

  // Ensure nextToggle is after now
  if (nextToggle <= tickMillis) {
    nextToggle = tickMillis + 1;
  }
}



void HapticPulser::setNextOnTime(unsigned long fixedDelay, unsigned long tickMillis) {
    drv.setRealtimeValue(128); // neutral
    state = OFF;
    lastStateChange = tickMillis;
    nextToggle = fixedDelay + tickMillis;

    // Guard: if the requested nextToggle is already <= now (race/wrap), push it slightly into the future
    if ((long)(nextToggle - lastStateChange) <= 0) {
      nextToggle = lastStateChange + 100; // keep off at least 100 ms
    }
}