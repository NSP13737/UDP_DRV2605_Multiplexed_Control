#include "HapticPulser.h"
#include <cmath> // For std::round()


// Assumes signed RTP input (127-255)
uint8_t pctToRtp(float percent) {
  if (percent <= 0.0f) return 127;
  if (percent >= 100.0f) return 255;
  return (uint8_t)(round((percent / 100.0f) * 128.0f)+127);
}

HapticPulser::HapticPulser(Adafruit_DRV2605 &d) : drv(d), state(IDLE) {}

bool HapticPulser::begin(float intensityPct_, unsigned long onMs_, unsigned long offMs_, bool doAutoCal, float ratedVoltage, float odClamp) {
  intensityPct = intensityPct_;
  onMs = onMs_;
  offMs = offMs_;
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
  unsigned long now = millis();
  if (state == IDLE) return;
  if (now < nextToggle) return;

  if (state == ON) {
    drv.setRealtimeValue(128); // neutral
    state = OFF;
    nextToggle = now + offMs;
  } else if (state == OFF) {
    drv.setRealtimeValue(pctToRtp(intensityPct));
    state = ON;
    nextToggle = now + onMs;
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
  if (state == ON) {
    nextToggle = millis() + onMs;
  } else if (state == OFF) {
    nextToggle = millis() + offMs;
  }
}

