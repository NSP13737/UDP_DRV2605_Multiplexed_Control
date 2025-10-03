#ifndef HAPTICPULSER_H
#define HAPTICPULSER_H

#include <Arduino.h>
#include "Adafruit_DRV2605.h"

// Utility function prototype (if you want it exposed here)
uint8_t pctToRtp(float percent);

class HapticPulser {
public:
  HapticPulser(Adafruit_DRV2605 &d);

  /**
   * @brief Initializes a pulser in an off state
   *
   * 
   *
   * @param 
   * 
   * @return true if auto-calibration completed successfully (or is doAutoCal=False), false otherwise.
   */
  bool begin(bool doAutoCal = false, float ratedVoltage = 1.0f, float odClamp = 1.5f);
  void start();
  void stop();
  void update();
  bool isOn();

  void setIntensity(float pct);
  void setOnOff(unsigned long onMs_, unsigned long offMs_);

private:
  Adafruit_DRV2605 &drv;
  enum { IDLE, ON, OFF } state;
  float intensityPct;
  unsigned long onMs, offMs;
  unsigned long nextToggle;

  /**
   * @brief Uses user set values from motor datasheet to get correct register value for programming the overdrive clamp register.
   *
   * 
   *
   * @param v_erm_clamp_volts Maximum voltage of ERM according to datasheet
   * 
   * @return value to be placed in overdrive clamp register for ERM.
   */
  uint8_t get_erm_od_clamp_reg(float v_erm_clamp_volts);
  
  /**
   * @brief Runs the DRV2605 auto-calibration routine for an ERM motor.
   *
   * This function configures the DRV2605 registers for auto-calibration,
   * starts the calibration sequence, waits for completion, and reports whether
   * the diagnostic succeeded. It also prints the resulting compensation and
   * back-EMF calibration values.
   *
   * @param ratedVoltage The motor’s rated voltage in volts (e.g., 3.0–4.0 V).
   * @param odClamp The maximum operating (overdrive) clamp voltage in volts.
   *
   * @return true if auto-calibration completed successfully, false otherwise.
   */
  bool runAutoCal(float ratedVoltage, float odClamp);


};

#endif
