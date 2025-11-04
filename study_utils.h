#ifndef STUDY_UTILS
#define STUDY_UTILS
//UDP_DRV2605_Multiplexed_Control.ino
#define FREQUENCY_CONDITION 1
#define DUTY_CYCLE_CONDITION 2
#define MIN_ACTIVATION_DIST 0.1f
#define MAX_ACTIVATION_DIST 1.0f


//belt_utils.h
#define NUM_DRIVERS 8
//freq modulation
#define MIN_FREQ_HZ 0.25
#define MAX_FREQ_HZ 25 //DO NOT SET ABOVE 50!
#define FIXED_DUTY_CYCLE 0.5
//duty cycle modulation
#define FIXED_PERIOD_MS 500 // 1000ms/2Hz

#endif