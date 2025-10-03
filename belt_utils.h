#ifndef BELTMANAGER
#define BELTMANAGER

#include <Arduino.h>
#include "HapticPulser.h"

#define NUM_DRIVERS 1
#define MULTIPLEX_ADDR 0x70

/**
    * @brief Used to select different I2C devices on multiplexer
    * @param i Which I2C device to select 0-7
    */
void multiplexSelect(uint8_t i);

bool setupBelt();

/**
   * @brief When new input is received, update belt accordingly 
   * @param received_distances C++ array with distances of all 8 measurements
   * @param participant_condition tells program how to control belt based on which condition participants are in
   * @return none
   */
void updateBelt(std::array<float,8> received_distances, int participant_condition);

void modulateIntensity(float distance, HapticPulser *pulser);





#endif