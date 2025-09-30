#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include "HapticPulser.h"

#define MULTIPLEX_ADDR 0x70
#define NUM_DRIVERS 1

Adafruit_DRV2605* drv[NUM_DRIVERS];
HapticPulser* pulser[NUM_DRIVERS];

//Below is for testing different vibrations
float intensity = 100.0f;
unsigned long offTimeMs = 20;
unsigned long onTimeMs = 20;
// Pulser cycle tracking using the pulser's own state
bool prevPulserOn = false;        // previous observed state from pulser.isOn()

void multiplexSelect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(MULTIPLEX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Program Start");

  for (int i = 0; i < NUM_DRIVERS; i++) {
    multiplexSelect(i);
    drv[i] = new Adafruit_DRV2605();
    // Start each drv
    if (!drv[i]->begin()) {
      Serial.println("Could not find DRV2605 #" + i);
      while(1) delay(10);
    }
    delay(50);
    // Create and start each pulser
    pulser[i] = new HapticPulser(*drv[i]);
    pulser[i]->begin(intensity, offTimeMs, onTimeMs, true, 3.8f, 4.0f);
    pulser[i]->start();
  }
  delay(50);
  
}

bool currentOn = false;
void loop() {

  for (int i = 0; i < NUM_DRIVERS; i++) {
    multiplexSelect(i);
    pulser[i]->update();
  }
  
  // //NOTE: Code below is for varying params during runtime

  // // Use the pulser's own reported state to detect a completed cycle (OFF -> ON)
  // currentOn = pulser.isOn();
  // // Detect transition from OFF to ON which indicates we completed an OFF period
  // if (currentOn && !prevPulserOn) {
  //   // completed a full cycle, update parameters
  //   intensity += 5.0f;
  //   onTimeMs += 0;
  //   offTimeMs += 0;

  //   // clamp/wrap values
  //   if (intensity > 100.0f) intensity = 0.0f;
  //   if (onTimeMs > 2000) onTimeMs = 100;
  //   if (offTimeMs > 2000) offTimeMs = 100;

  //   // apply the updated params to the pulser
  //   pulser.setIntensity(intensity);
  //   pulser.setOnOff(onTimeMs, offTimeMs);
  //   Serial.println("Intensity: " + String(intensity) +
  //              " On: " + String(onTimeMs) +
  //              " Off: " + String(offTimeMs));
  //   Serial.flush();
  // }

  // prevPulserOn = currentOn;

  
  

    
  
}
