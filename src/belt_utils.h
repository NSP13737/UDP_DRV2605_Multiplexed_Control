#ifndef BELTMANAGER
#define BELTMANAGER

#include <Arduino.h>
#include "HapticPulser.h"
#include "enums.h"

#define NUM_DRIVERS 8
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
void updateBelt(std::array<float,8>& distances, std::array<float,8>& study_params);

/**
 * @brief Using quatratic function: Takes the users raw distance from a wall and converts it to a percentage to be used by modulation helpers
 * @param distance The raw distance recieved from Unity
 * @param maxActivationDist The distance the tactor should be "weakest" at
 * @param minActivationDist The distance where the tactor should be "strongest"
 */
float rawDistToActivationPercentage(float distance, float min_activation_dist, float max_activation_dist);

/**
 * @brief Directly modulates intensity of pulser from 0-100
 * @param activation_percentage
 * @param pulser pulser to modulate
 * @param just_detectable_intensity A percentage which sets the minimum intensity to be the minimum that each user can feel
 */
void modulateIntensity(float activation_percentage, HapticPulser *pulser, float just_detectable_intensity);

/**
 * @brief Modulates how quickly the pulser pulses
 * @param activation_percentage
 * @param pulser pulser to modulate
 * @param min_freq_hz the "slowest" the pulses should be. Occurs when user is >= max_activation_dist from wall
 * @param min_freq_hz the "fastest" the pulses should be. Occurs when user is <= min_activation_dist from wall
 * @param fixed_duty_cycle Controls the duty cycle of each pulse, which remains fixed across freqs
 */
void modulatePulseFrequency(float activation_percentage, HapticPulser *pulser, float min_freq_hz, float max_freq_hz, float fixed_duty_cycle);

/**
 * @brief Converts hz to period in Ms. If hz is 0, period is also set to 0 (to avoid div by 0)
 */
unsigned long hzToPeriodMs(float hz);


#endif