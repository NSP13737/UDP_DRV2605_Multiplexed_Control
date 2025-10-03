#include "belt_utils.h"

Adafruit_DRV2605* drv[NUM_DRIVERS];
HapticPulser* pulser[NUM_DRIVERS];

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

void updateBelt(char *input) {
    
    for (int i = 0; i < NUM_DRIVERS; i++) {
    multiplexSelect(i);
    pulser[i]->update();
    }
}

// void updateBelt(char *input) {
//     // Temporary debug: on every rising edge (OFF -> ON) increase intensity by 10%,
//     // wrap to 10% after 100%, and apply via setIntensity().
//     static bool prevOn[NUM_DRIVERS] = { false };
//     static float debugIntensity[NUM_DRIVERS];
//     static bool initialized = false;

//     if (!initialized) {
//         for (int i = 0; i < NUM_DRIVERS; ++i) {
//             debugIntensity[i] = 10.0f; // start at 10%
//             prevOn[i] = false;
//         }
//         initialized = true;
//     }

//     for (int i = 0; i < NUM_DRIVERS; i++) {
//         multiplexSelect(i);
//         pulser[i]->update();

//         bool nowOn = pulser[i]->isOn();

//         // Detect rising edge: just turned ON
//         if (nowOn && !prevOn[i]) {
//             debugIntensity[i] += 10.0f;
//             if (debugIntensity[i] > 100.0f) debugIntensity[i] = 10.0f;
//             pulser[i]->setIntensity(debugIntensity[i]);
//             Serial.print("DEBUG: driver ");
//             Serial.print(i);
//             Serial.print(" intensity -> ");
//             Serial.println(debugIntensity[i]);
//         }

//         prevOn[i] = nowOn;
//     }
// }