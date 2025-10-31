#ifndef BELTMANAGER
#define BELTMANAGER

#include <Arduino.h>
#include "HapticPulser.h"
#include "study_utils.h"

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
   * @param participant_condition 1 = duty cycle modulation; 2 = total pulse time modulation
   * @return none
*/
void updateBelt(std::array<float,8> receivedDistances, int participantCondition, float minActivationDist, float maxActivationDist);

/**
 * @brief Using quatratic function: Takes the users raw distance from a wall and converts it to a percentage to be used by modulation helpers
 * @param distance The raw distance recieved from Unity
 * @param maxActivationDist The distance the tactor should be "weakest" at
 * @param minActivationDist The distance where the tactor should be "strongest"
 */
float rawDistToActivationPercentage(float distance, float minActivationDist, float maxActivationDist);

void modulateIntensity(float activationPercentage, HapticPulser *pulser);

void modulatePulseFrequency(float activationPercentage, HapticPulser *pulser);

/**
   * @brief 
   * @return none
*/
void modulatePulseDutyCycle(float activationPercentage, HapticPulser *pulser);


#endif