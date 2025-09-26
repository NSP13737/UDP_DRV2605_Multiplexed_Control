#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include "HapticPulser.h"

Adafruit_DRV2605 drv;

// ---------- Global Pulser ----------
HapticPulser pulser(drv);

//Below is for testing different vibrations
float intensity = 10.0f;
unsigned long offTimeMs = 700;
unsigned long onTimeMs = 700;
// Pulser cycle tracking using the pulser's own state
bool prevPulserOn = false;        // previous observed state from pulser.isOn()

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Program Start");

  if (!drv.begin()) {
    Serial.println("DRV2605 not found");
    while (1) delay(10);
  }
  delay(50);
  Serial.println("DRV2605 found");
  
  //SETUP: Closed-loop unidirectional mode

  //NOTE: In closed loop uni-directional, Control 3 should be set to unsigned (see datasheet 7.5.8.1.2)
  // Configure Control3 for unsigned RTP values
  uint8_t ctrl3 = drv.readRegister8(DRV2605_REG_CONTROL3);
  ctrl3 |= (1 << 3); // DATA_FORMAT_RTP = unsigned
  drv.writeRegister8(DRV2605_REG_CONTROL3, ctrl3);

  uint8_t ctrl2 = drv.readRegister8(DRV2605_REG_CONTROL2);
  ctrl2 &= ~(1 << 7); //Setting 7th bit (BiDir_Input) to 0
  drv.writeRegister8(DRV2605_REG_CONTROL2, ctrl2);

  pulser.begin(intensity, offTimeMs, onTimeMs, true, 3.8f, 4.0f);
  pulser.start();
}

bool currentOn = false;
void loop() {
  pulser.update();



  //NOTE: Code below is for varying params during runtime

  // Use the pulser's own reported state to detect a completed cycle (OFF -> ON)
  currentOn = pulser.isOn();
  // Detect transition from OFF to ON which indicates we completed an OFF period
  if (currentOn && !prevPulserOn) {
    // completed a full cycle, update parameters
    intensity += 5.0f;
    onTimeMs += 0;
    offTimeMs += 0;

    // clamp/wrap values
    if (intensity > 100.0f) intensity = 0.0f;
    if (onTimeMs > 2000) onTimeMs = 100;
    if (offTimeMs > 2000) offTimeMs = 100;

    // apply the updated params to the pulser
    pulser.setIntensity(intensity);
    pulser.setOnOff(onTimeMs, offTimeMs);
    Serial.println("Intensity: " + String(intensity) +
               " On: " + String(onTimeMs) +
               " Off: " + String(offTimeMs));
    Serial.flush();
  }

  prevPulserOn = currentOn;

  
  

    
  
}
