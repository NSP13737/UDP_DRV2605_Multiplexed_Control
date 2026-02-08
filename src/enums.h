#ifndef ENUMS_H
#define ENUMS_H

enum PulserState {
    IDLE,
    ON,
    OFF,
};

enum ConditionState {
    NONE = 0,
    FREQUENCY_MODULATION = 1,
    DUTY_CYCLE_MODULATION = 2,
};

#endif