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
   * @param command string with command and inputs
   * @return none
   */
void updateBelt(char *input);





#endif