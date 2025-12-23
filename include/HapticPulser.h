#ifndef HAPTICPULSER_H
#define HAPTICPULSER_H

#include <Arduino.h>
#include "Adafruit_DRV2605.h"
#include "enums.h"

// Utility function prototype (if you want it exposed here)
uint8_t pctToRtp(float percent);

class HapticPulser {
public:
  HapticPulser(Adafruit_DRV2605 &d, uint8_t id);

  /**
   * @brief Initializes a pulser in an off state
   *
   * @param 
   * 
   * @return true if auto-calibration completed successfully (or is doAutoCal=False), false otherwise.
   */
  bool begin(bool doAutoCal = false, float ratedVoltage = 1.0f, float odClamp = 1.5f);

  /**
   * @brief Starts pulser
   * @param tickMillis Fixed time sent to all pulsers (so they are all acting on same ticks)
   */
  void start(unsigned long tickMillis);

  void stop();

  /**
   * @brief Check if pulser needs updating, if so, update
   * @param tickMillis Fixed time sent to all pulsers (so they are all acting on same ticks)
   */
  void update(unsigned long tickMillis);

  bool isOn();

  void setIntensity(float pct);

  /**
   * @brief Sets how long pulser should be on and off for
   * @param desiredState
   */
  void setState(PulserState desiredState);

  /**
   * @brief Sets how long pulser should be on and off for
   * @param tickMillis Fixed time sent to all pulsers (so they are all acting on same ticks)
   */
  void setOnOff(unsigned long onMs_, unsigned long offMs_, unsigned long tickMillis);

  /**
   * @brief Sets next time for motor to turn on at
   * 
   * Will reset the state of the pulser to off before setting the next on time
   *
   * @param fixedDelay Added delay (delta time in future to set next on time)
   * @param tickMillis Fixed time sent to all pulsers (so they are all acting on same ticks)
   */
  void setNextOnTime(unsigned long fixedDelay, unsigned long tickMillis);

private:
  Adafruit_DRV2605 &drv;
  uint8_t motorId;
  PulserState state;
  float intensityPct;
  unsigned long onMs, offMs;
  unsigned long nextToggle;
  unsigned long lastStateChange;

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
